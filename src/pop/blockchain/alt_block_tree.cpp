// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/alt_block_tree_util.hpp>
#include <veriblock/pop/command_group_cache.hpp>
#include <veriblock/pop/entities/context_info_container.hpp>
#include <veriblock/pop/reversed_range.hpp>

namespace altintegration {

template struct BlockIndex<AltBlock>;
template struct BaseBlockTree<AltBlock>;

template <>
bool checkBlockTime(const BlockIndex<AltBlock>& prev,
                    const AltBlock& block,
                    ValidationState& state,
                    const AltChainParams& params) {
  (void)prev;
  int64_t blockTime = block.getBlockTime();
  int64_t maxTime = currentTimestamp4() + params.maxAltchainFutureBlockTime();
  if (blockTime > maxTime) {
    return state.Invalid("alt-time-too-new",
                         "ALT block timestamp too far in the future");
  }

  return true;
}

bool AltBlockTree::bootstrap(ValidationState& state) {
  if (base::isBootstrapped()) {
    return state.Invalid("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto height = block.getHeight();
  VBK_ASSERT_MSG(
      height >= 0,
      "AltBlockTree can be bootstrapped with block height >= 0, got=%d",
      height);
  auto max = std::numeric_limits<block_height_t>::max() / 2;
  VBK_ASSERT_MSG(
      height < max,
      "AltBlockTree can be bootstrapped with block height <= %d, got=%d",
      max,
      height);

  auto* index =
      insertBlockHeader(std::make_shared<AltBlock>(std::move(block)), height);
  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  height = index->getHeight();
  index->setFlag(BLOCK_ACTIVE);
  ++appliedBlockCount;
  index->setFlag(BLOCK_BOOTSTRAP);
  index->raiseValidity(BLOCK_CAN_BE_APPLIED);

  base::activeChain_ = Chain<index_t>(height, index);

  VBK_ASSERT(base::isBootstrapped());

  VBK_ASSERT(getBlockIndex(index->getHash()) != nullptr &&
             "getBlockIndex must be able to find the block added by "
             "insertBlockHeader");

  tryAddTip(index);

  return true;
}

template <typename Pop>
void commitPayloadsIds(BlockIndex<AltBlock>& index,
                       const std::vector<Pop>& pop,
                       PayloadsIndex& storage) {
  auto pids = map_get_id(pop);
  index.template setPayloads<Pop>(pids);

  auto containing = index.getHash();
  for (const auto& pid : pids) {
    storage.addAltPayloadIndex(containing, pid.asVector());
  }
}

template <typename P, typename T>
bool hasDuplicates(BlockIndex<AltBlock>& index,
                   std::vector<T> payloads,
                   AltBlockTree& tree,
                   ValidationState& state) {
  auto duplicates = findDuplicates<T>(index, payloads, tree);
  return duplicates == payloads.end()
             ? false
             : !state.Invalid(
                   P::name() + "-duplicate",
                   fmt::format("Found {} duplicate {}",
                               std::distance(duplicates, payloads.end()),
                               P::name()));
}

void AltBlockTree::acceptBlock(const hash_t& block, const PopData& payloads) {
  auto* index = getBlockIndex(block);
  VBK_ASSERT_MSG(index, "cannot find block %s", HexStr(block));
  return acceptBlock(*index, payloads);
}

void AltBlockTree::acceptBlock(index_t& index, const PopData& payloads) {
  ValidationState dummy;
  acceptBlock(index, payloads, dummy);
}

void AltBlockTree::acceptBlock(index_t& index,
                               const PopData& payloads,
                               ValidationState& state) {
  setPayloads(index, payloads);

  if (index.pprev->isConnected()) {
    connectBlock(index, state);
  }
}

void AltBlockTree::setPayloads(index_t& index, const PopData& payloads) {
  VBK_LOG_DEBUG("%s add %s to block %s",
                block_t::name(),
                payloads.toPrettyString(),
                index.toShortPrettyString());

  VBK_ASSERT_MSG(index.isValidUpTo(BLOCK_VALID_TREE),
                 "block %s should be valid",
                 index.toPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_HAS_PAYLOADS),
                 "block %s already contains payloads",
                 index.toPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_ACTIVE),
                 "state corruption: block %s is applied",
                 index.toPrettyString());

  VBK_ASSERT_MSG(index.pprev,
                 "Adding payloads to a bootstrap block is not allowed");

  ValidationState state;
  VBK_ASSERT_MSG(
      checkPopDataForDuplicates(payloads, state),
      "attempted to add statelessly invalid payloads to block %s: %s",
      index.toPrettyString(),
      state.toString());

  // add payload ids to the block, update the payload index
  commitPayloadsIds<VbkBlock>(index, payloads.context, payloadsIndex_);
  commitPayloadsIds<VTB>(index, payloads.vtbs, payloadsIndex_);
  commitPayloadsIds<ATV>(index, payloads.atvs, payloadsIndex_);

  payloadsProvider_.writePayloads(payloads);

  // we successfully added this block payloads
  index.setFlag(BLOCK_HAS_PAYLOADS);
}

bool AltBlockTree::connectBlock(index_t& index, ValidationState& state) {
  VBK_ASSERT_MSG(index.hasFlags(BLOCK_HAS_PAYLOADS),
                 "block %s must have payloads added",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!index.isConnected(),
                 "block %s must not be connected",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_ACTIVE),
                 "state corruption: block %s is applied",
                 index.toPrettyString());
  VBK_ASSERT_MSG(index.pprev->isConnected(),
                 "the previous block of block %s must be connected",
                 index.toPrettyString());
  VBK_ASSERT_MSG(index.allDescendantsUnconnected(),
                 "a descendant of block %s is connected",
                 index.toPrettyString());

  bool success = index.raiseValidity(BLOCK_CONNECTED);
  VBK_ASSERT(success);

  // partial stateful validation
  // FIXME: eventually we want to perform full stateful validation here,
  // effectively find out whether setState(index) will be successful

  if (hasDuplicates<VbkBlock>(
          index, index.getPayloadIds<VbkBlock>(), *this, state) ||
      hasDuplicates<VTB>(index, index.getPayloadIds<VTB>(), *this, state) ||
      hasDuplicates<ATV>(index, index.getPayloadIds<ATV>(), *this, state)) {
    invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
  }

  tryAddTip(&index);

  if (index.isValid()) {
    onBlockConnected.emit(index);
    // connect the descendants
    for (auto* successor : index.pnext) {
      if (successor->hasFlags(BLOCK_HAS_PAYLOADS)) {
        ValidationState dummy;
        connectBlock(*successor, dummy);
      }
    }
  } else {
    onInvalidBlockConnected.emit(index, state);
  };

  return index.isValid();
}

bool AltBlockTree::acceptBlockHeader(const AltBlock& block,
                                     ValidationState& state) {
  // We don't calculate hash of AltBlock, thus users may call acceptBlockHeader
  // with AltBlock, where hash == previousHash. If so, fail loudly.
  assertBlockSanity(block);

  // Handle edge case:
  // If block tree is bootstrapped with genesis block, its 'prev' does not
  // exist. When we execute 'acceptBlockHeader' on genesis block again, we fail
  // that block validation, because it's previous block is not known.
  if (isBootstrapped() && block.getHash() == activeChain_.first()->getHash()) {
    return true;
  }

  // we must know previous block, but not if `block` is bootstrap block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid(
        block_t::name() + "-bad-prev-block",
        "can not find previous block: " + HexStr(block.previousBlock));
  }

  auto* index = insertBlockHeader(std::make_shared<AltBlock>(block));

  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  if (!index->isValid()) {
    return state.Invalid(
        block_t::name() + "-bad-chain",
        fmt::format("One of previous blocks is invalid. Status=({})",
                    index->getStatus()));
  }

  tryAddTip(index);

  return true;
}

std::string AltBlockTree::toPrettyString(size_t level) const {
  std::string pad(level, ' ');
  return fmt::sprintf("%sAltTree{blocks=%llu\n%s\n%s\n%s}",
                      pad,
                      base::getBlocks().size(),
                      base::toPrettyString(level + 2),
                      cmp_.toPrettyString(level + 2),
                      pad);
}

void AltBlockTree::determineBestChain(index_t& candidate, ValidationState&) {
  auto bestTip = getBestChain().tip();
  VBK_ASSERT(bestTip && "must be bootstrapped");

  if (bestTip == &candidate) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!candidate.isValid()) {
    VBK_LOG_DEBUG("Candidate %s is invalid, skipping FR",
                  candidate.toPrettyString());
    return;
  }

  // if tip==nullptr, then update tip.
  // else - do nothing. AltTree does not (yet) do fork resolution
}

int AltBlockTree::comparePopScore(const AltBlock::hash_t& A,
                                  const AltBlock::hash_t& B) {
  auto* left = getBlockIndex(A);
  auto* right = getBlockIndex(B);
  VBK_ASSERT_MSG(left, "unknown 'A' block %s", HexStr(A));
  VBK_ASSERT_MSG(right, "unknown 'B' block %s", HexStr(B));
  VBK_ASSERT(activeChain_.tip() && "not bootstrapped");
  VBK_ASSERT_MSG(activeChain_.tip() == left,
                 "left fork must be applied. Tip: %s, Left: %s",
                 activeChain_.tip()->toPrettyString(),
                 left->toPrettyString());

  VBK_ASSERT_MSG(left->isValidUpTo(BLOCK_CONNECTED), "A is not connected");
  VBK_ASSERT_MSG(right->isValidUpTo(BLOCK_CONNECTED), "B is not connected");

  ValidationState state;
  // compare current active chain to other chain
  int result = cmp_.comparePopScore(*this, *right, state);
  if (result < 0) {
    // other chain is better, and we already changed 'cmp' state to winner, so
    // just update active chain tip
    activeChain_.setTip(right);
  }

  return result;
}

template <typename Pop, typename Index>
static void clearSideEffects(Index& index, PayloadsIndex& storage) {
  const auto& containingHash = index.getHash();
  auto& payloadIds = index.template getPayloadIds<Pop>();
  for (const auto& pid : payloadIds) {
    storage.removeAltPayloadIndex(containingHash, pid.asVector());
  }
}

void AltBlockTree::removeAllPayloads(index_t& index) {
  VBK_LOG_INFO("%s remove VBK=%d VTB=%d ATV=%d payloads from %s",
               block_t::name(),
               index.getPayloadIds<VbkBlock>().size(),
               index.getPayloadIds<VTB>().size(),
               index.getPayloadIds<ATV>().size(),
               index.toShortPrettyString());

  // we do not allow adding payloads to the genesis block
  VBK_ASSERT_MSG(index.pprev, "can not remove payloads from the genesis block");
  VBK_ASSERT_MSG(index.hasFlags(BLOCK_HAS_PAYLOADS),
                 "Can remove payloads only from blocks with payloads");
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_ACTIVE), "block is applied");

  VBK_ASSERT_MSG(index.allDescendantsUnconnected(),
                 "can remove payloads only from connected leaves");

  if (index.hasPayloads()) {
    clearSideEffects<VbkBlock>(index, payloadsIndex_);
    clearSideEffects<VTB>(index, payloadsIndex_);
    clearSideEffects<ATV>(index, payloadsIndex_);
    index.clearPayloads();
  }

  VBK_ASSERT(!index.hasPayloads());

  index.unsetFlag(BLOCK_HAS_PAYLOADS);

  revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);

  // allow removing payloads from unconnected blocks
  if (index.isConnected()) {
    bool success = index.lowerValidity(BLOCK_VALID_TREE);
    VBK_ASSERT(success);
  }

  // the current block is no longer a tip
  tips_.erase(&index);
  // the previous block might have become a tip
  tryAddTip(index.pprev);
}

bool AltBlockTree::setState(index_t& to, ValidationState& state) {
  VBK_ASSERT_MSG(
      to.isConnected(), "block %s must be connected", to.toPrettyString());

  bool success = cmp_.setState(*this, to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }
  return success;
}

void AltBlockTree::overrideTip(index_t& to) {
  VBK_LOG_DEBUG("ALT=\"%s\", VBK=\"%s\", BTC=\"%s\"",
                to.toShortPrettyString(),
                (vbk().getBestChain().tip()
                     ? vbk().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"),
                (btc().getBestChain().tip()
                     ? btc().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"));

  VBK_ASSERT_MSG(to.isValid(BLOCK_CAN_BE_APPLIED),
                 "the active chain tip(%s) must be fully valid",
                 to.toPrettyString());

  onBeforeOverrideTip.emit(to);
  activeChain_.setTip(&to);
}

bool AltBlockTree::loadBlock(const stored_index_t& index,
                             ValidationState& state) {
  if (!base::loadBlock(index, state)) {
    return false;  // already set
  }

  // load endorsements
  const auto& containingHash = index.header->getHash();
  auto* current = getBlockIndex(containingHash);
  VBK_ASSERT(current);

  const auto& vbkblockIds = current->getPayloadIds<VbkBlock>();
  const auto& vtbIds = current->getPayloadIds<VTB>();
  const auto& atvIds = current->getPayloadIds<ATV>();

  // stateless check for duplicates in each of the payload IDs vectors
  if (!checkIdsForDuplicates<VbkBlock>(vbkblockIds, state)) return false;
  if (!checkIdsForDuplicates<VTB>(vtbIds, state)) return false;
  if (!checkIdsForDuplicates<ATV>(atvIds, state)) return false;

  if (hasDuplicates<VbkBlock>(*current, vbkblockIds, *this, state) ||
      hasDuplicates<VTB>(*current, vtbIds, *this, state) ||
      hasDuplicates<ATV>(*current, atvIds, *this, state)) {
    return false;
  }

  // recover `endorsedBy` and `blockOfProofEndorsements`
  const int si = getParams().getEndorsementSettlementInterval();
  auto window = std::max(0, index.height - si);
  Chain<index_t> chain(window, current);
  if (!recoverEndorsements(*this, chain, *current, state)) {
    return state.Invalid("bad-endorsements");
  }

  payloadsIndex_.addBlockToIndex(*current);

  return true;
}

AltBlockTree::AltBlockTree(const AltBlockTree::alt_config_t& alt_config,
                           const AltBlockTree::vbk_config_t& vbk_config,
                           const AltBlockTree::btc_config_t& btc_config,
                           PayloadsStorage& payloadsProvider)
    : alt_config_(&alt_config),
      cmp_(std::make_shared<VbkBlockTree>(
               vbk_config, btc_config, payloadsProvider, payloadsIndex_),
           alt_config,
           payloadsProvider,
           payloadsIndex_),
      payloadsProvider_(payloadsProvider) {}

void AltBlockTree::removeSubtree(AltBlockTree::index_t& toRemove) {
  payloadsIndex_.removePayloadsIndex(toRemove);
  base::removeSubtree(toRemove);
}

bool AltBlockTree::loadTip(const AltBlockTree::hash_t& hash,
                           ValidationState& state) {
  if (!base::loadTip(hash, state)) {
    return false;
  }

  auto* tip = activeChain_.tip();
  VBK_ASSERT(tip);
  appliedBlockCount = 0;
  while (tip != nullptr) {
    tip->setFlag(BLOCK_ACTIVE);
    ++appliedBlockCount;
    tip->raiseValidity(BLOCK_CAN_BE_APPLIED);
    tip = tip->pprev;
  }

  return true;
}

void AltBlockTree::removePayloads(const AltBlockTree::hash_t& hash) {
  auto* index = getBlockIndex(hash);
  VBK_ASSERT_MSG(index, "does not contain block %s", HexStr(hash));
  return removeAllPayloads(*index);
}

std::vector<const AltBlockTree::index_t*> AltBlockTree::getConnectedTipsAfter(
    const AltBlockTree::index_t& index) const {
  std::vector<const index_t*> candidates;

  if (!index.isConnected()) {
    // this block is not connected
    return candidates;
  }

  for (auto* tip : getTips()) {
    if (tip->isConnected() && tip->getAncestor(index.getHeight()) == &index) {
      // tip is a successor of index and it is connected
      candidates.push_back(tip);
    }
  }

  return candidates;
}

bool AltBlockTree::finalizeBlock(const AltBlockTree::hash_t& block) {
  return finalizeBlockImpl(block, getParams().preserveBlocksBehindFinal());
}

template <typename Payloads>
void removeId(PayloadsIndex& storage,
              BlockIndex<AltBlock>& index,
              const typename Payloads::id_t& pid) {
  auto& payloads = index.template getPayloadIds<Payloads>();
  auto it = std::find(payloads.rbegin(), payloads.rend(), pid);
  VBK_ASSERT(it != payloads.rend());
  index.removePayloadId<Payloads>(pid);
  storage.removeAltPayloadIndex(index.getHash(), pid.asVector());
}

template <>
void removePayloadsFromIndex(PayloadsIndex& storage,
                             BlockIndex<AltBlock>& index,
                             const CommandGroup& cg) {
  // TODO: can we do better?
  if (cg.payload_type_name == &VTB::name()) {
    removeId<VTB>(storage, index, cg.id);
    return;
  }

  if (cg.payload_type_name == &ATV::name()) {
    removeId<ATV>(storage, index, cg.id);
    return;
  }

  if (cg.payload_type_name == &VbkBlock::name()) {
    removeId<VbkBlock>(storage, index, cg.id);
    return;
  }

  VBK_ASSERT(false && "should not reach here");
}

}  // namespace altintegration
