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

void AltBlockTree::acceptBlock(const hash_t& block, const PopData& payloads) {
  auto* index = getBlockIndex(block);
  VBK_ASSERT_MSG(index, "cannot find block %s", HexStr(block));
  return acceptBlock(*index, payloads);
}

void AltBlockTree::acceptBlock(index_t& index, const PopData& payloads) {
  VBK_LOG_INFO("Accept pop data: %s, for the block: %s ",
               payloads.toPrettyString(),
               index.toPrettyString());
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

template <typename Payload>
bool hasStatefulDuplicates(AltBlockTree::BlockPayloadMutator& mutator,
                           ValidationState& state) {
  for (const auto& pid : mutator.getBlock().getPayloadIds<Payload>()) {
    if (mutator.isStatefulDuplicate(pid.asVector())) {
      return !state.Invalid(
          Payload ::name() + "-duplicate",
          fmt::format("Payload {}:{} in block {} is a stateful duplicate",
                      Payload::name(),
                      pid.toHex(),
                      mutator.getBlock().toPrettyString()));
    }
  }

  return false;
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

  auto mutator = makeConnectedLeafPayloadMutator(index);

  if (hasStatefulDuplicates<VbkBlock>(mutator, state) ||
      hasStatefulDuplicates<VTB>(mutator, state) ||
      hasStatefulDuplicates<ATV>(mutator, state)) {
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
  VBK_LOG_INFO("Accept new block: %s ", block.toPrettyString());

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

  VBK_ASSERT_MSG(index != nullptr,
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
  auto* bestTip = getBestChain().tip();
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
  VBK_LOG_INFO(
      "Compare two chains. chain A: %s, chain B: %s", HexStr(A), HexStr(B));

  auto* left = getBlockIndex(A);
  auto* right = getBlockIndex(B);

  VBK_ASSERT_MSG(left, "unknown 'A' block %s", HexStr(A));
  VBK_ASSERT(activeChain_.tip() && "not bootstrapped");
  VBK_ASSERT_MSG(activeChain_.tip() == left,
                 "left fork must be applied. Tip: %s, Left: %s",
                 activeChain_.tip()->toPrettyString(),
                 left->toPrettyString());
  if (right == nullptr) {
    VBK_LOG_WARN(
        "Unknown 'B block: %s. Maybe you have forgotten to execute "
        "acceptBlockHeader and acceptBlock on such block.",
        HexStr(B));
    return 1;
  }

  VBK_ASSERT_MSG(left->isValidUpTo(BLOCK_CONNECTED), "A is not connected");
  VBK_ASSERT_MSG(right->isValidUpTo(BLOCK_CONNECTED), "B is not connected");

  ValidationState state;
  // compare current active chain to other chain
  int result = cmp_.comparePopScore(*right, state);
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

template <typename Payload, typename Block>
void appendIds(std::unordered_set<std::vector<uint8_t>>& ids,
               const Block& block) {
  for (const auto& id : block.template getPayloadIds<Payload>()) {
    ids.emplace(id.asVector());
  }
}

AltBlockTree::BlockPayloadMutator::BlockPayloadMutator(tree_t& tree,
                                                       block_index_t& block)
    : tree_(tree),
      block_(block),
      payload_index_(tree.getPayloadsIndex()),
      // don't look for duplicates in the block itself
      chain_(tree.getParams().getBootstrapBlock().height, block_.pprev) {
  VBK_ASSERT_MSG(block_.pprev,
                 "Adding payloads to a bootstrap block is not allowed");
  VBK_ASSERT_MSG(block_.isValidUpTo(BLOCK_VALID_TREE),
                 "block %s should be valid",
                 block_.toPrettyString());

  VBK_ASSERT_MSG(block_.isConnected(),
                 "block %s must be connected",
                 block_.toPrettyString());

  // an expensive check
  VBK_ASSERT_MSG(block_.allDescendantsUnconnected(),
                 "a descendant of block %s is connected",
                 block_.toPrettyString());

  appendIds<ATV>(ids_, block_);
  appendIds<VTB>(ids_, block_);
  appendIds<VbkBlock>(ids_, block_);
}

bool AltBlockTree::BlockPayloadMutator::isStatefulDuplicate(
    const id_vector_t& payload_id) {
  // make sure existing blocks do not contain this id
  for (const auto& hash : payload_index_.getContainingAltBlocks(payload_id)) {
    const auto* candidate = tree_.getBlockIndex(hash);
    if (chain_.contains(candidate)) {
      // duplicate found in 'candidate'
      return true;
    }
  }

  return false;
}

bool AltBlockTree::BlockPayloadMutator::isStatelessDuplicate(
    const id_vector_t& payload_id) {
  return ids_.count(payload_id) > 0;
}

template <typename Payload>
bool AltBlockTree::BlockPayloadMutator::add(const Payload& payload,
                                            ValidationState& state) {
  auto pid = payload.getId();
  VBK_LOG_DEBUG("Adding and applying payload %s in block %s",
                pid.toHex(),
                block_.toShortPrettyString());

  VBK_ASSERT_MSG(
      !isStatelessDuplicate(pid.asVector()),
      "attempted to add a statelessly duplicate payload %s to block %s",
      pid.toHex(),
      block_.toShortPrettyString());

  if (isStatefulDuplicate(pid.asVector())) {
    return state.Invalid(
        Payload ::name() + "-duplicate",
        fmt::format("Payload {}:{} in block {} is a stateful duplicate",
                    Payload::name(),
                    pid.toHex(),
                    block_.toShortPrettyString()));
  }

  block_.insertPayloadIds<Payload>({pid});
  payload_index_.addAltPayloadIndex(block_.getHash(), pid.asVector());

  bool inserted = ids_.insert(pid.asVector()).second;
  VBK_ASSERT(inserted);

  if (!block_.hasFlags(BLOCK_ACTIVE)) return true;

  auto cgroup =
      tree_.getCommandGroupStore().getCommand<Payload>(block_, pid, state);

  if (!cgroup || !cgroup->execute(state)) {
    VBK_LOG_DEBUG("%s cannot be added to block %s: %s",
                  payload.toPrettyString(),
                  block_.toShortPrettyString(),
                  state.toString());

    // remove the failed payload
    block_.removePayloadId<Payload>(pid);
    payload_index_.removeAltPayloadIndex(block_.getHash(), pid.asVector());

    return false;
  }

  return true;
}

template bool AltBlockTree::BlockPayloadMutator::add(const ATV& payload,
                                                     ValidationState& state);

template bool AltBlockTree::BlockPayloadMutator::add(const VTB& payload,
                                                     ValidationState& state);

template bool AltBlockTree::BlockPayloadMutator::add(const VbkBlock& payload,
                                                     ValidationState& state);

AltBlockTree::BlockPayloadMutator AltBlockTree::makeConnectedLeafPayloadMutator(
    index_t& block) {
  return {*this, block};
}

bool AltBlockTree::setState(index_t& to, ValidationState& state) {
  VBK_ASSERT_MSG(
      to.isConnected(), "block %s must be connected", to.toPrettyString());

  bool success = cmp_.setState(to, state);
  if (success) {
    overrideTip(to);
    // finalize blocks
    {
      auto* bestTip = getBestChain().tip();
      VBK_ASSERT(bestTip && "must be bootstrapped");

      uint32_t max_reorg_distance = getParams().getMaxReorgDistance();
      uint32_t finalHeight =
          std::max((int32_t)(bestTip->getHeight() - max_reorg_distance),
                   (int32_t)getRoot().getHeight());

      auto* finalizedBlock = getBestChain()[finalHeight];

      if (!finalizeBlock(*finalizedBlock, state)) {
        return state.Invalid("set-state-error");
      }
    }
  } else {
    VBK_ASSERT_MSG(!to.isValid(),
                   "if setState failed, then '%s must be invalid",
                   to.toShortPrettyString());
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

bool AltBlockTree::loadBlockForward(const stored_index_t& index,
                                    ValidationState& state) {
  if (!base::loadBlockForward(index, state)) {
    return false;  // already set
  }
  return loadBlockInner(index, state);
}

bool AltBlockTree::loadBlockBackward(const stored_index_t& index,
                                     ValidationState& state) {
  if (!base::loadBlockBackward(index, state)) {
    return false;
  }
  return loadBlockInner(index, state);
}

bool AltBlockTree::loadBlockInner(const stored_index_t& index,
                                  ValidationState& state) {
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

  if (current->pprev != nullptr) {
    // if the block is not yet connected, defer the stateful duplicate check to
    // connectBlock()
    if (current->isConnected()) {
      auto mutator = makeConnectedLeafPayloadMutator(*current);
      if (hasStatefulDuplicates<VbkBlock>(mutator, state) ||
          hasStatefulDuplicates<VTB>(mutator, state) ||
          hasStatefulDuplicates<ATV>(mutator, state)) {
        return false;
      }
    }
  } else {
    VBK_ASSERT_MSG(!current->hasPayloads(),
                   "the bootstrap block is not allowed to have payloads");
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
                           PayloadsStorage& payloadsProvider,
                           BlockReader& blockProvider)
    : base(blockProvider),
      alt_config_(&alt_config),
      cmp_(*this,
           std::make_shared<VbkBlockTree>(vbk_config,
                                          btc_config,
                                          payloadsProvider,
                                          blockProvider,
                                          payloadsIndex_),
           alt_config,
           payloadsProvider,
           payloadsIndex_),
      payloadsProvider_(payloadsProvider),
      commandGroupStore_(*this, payloadsProvider_) {}

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

bool AltBlockTree::finalizeBlock(index_t& index, ValidationState& state) {
  return this->finalizeBlockImpl(
      index, getParams().preserveBlocksBehindFinal(), state);
}

bool AltBlockTree::finalizeBlockImpl(index_t& index,
                                     int32_t preserveBlocksBehindFinal,
                                     ValidationState& state) {
  auto* bestVbkTip = vbk().getBestChain().tip();
  VBK_ASSERT(bestVbkTip && "VBK tree must be bootstrapped");

  int32_t firstBlockHeight =
      bestVbkTip->getHeight() - vbk().getParams().getOldBlocksWindow();
  int32_t bootstrapBlockHeight = vbk().getRoot().getHeight();
  firstBlockHeight = std::max(bootstrapBlockHeight, firstBlockHeight);
  auto* finalizedIndex = vbk().getBestChain()[firstBlockHeight];
  VBK_ASSERT_MSG(finalizedIndex != nullptr, "Invalid VBK tree state");
  if (!vbk().finalizeBlock(*finalizedIndex, state)) {
    return state.Invalid("vbktree-finalize-error");
  }
  return base::finalizeBlockImpl(index, preserveBlocksBehindFinal, state);
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

  // fix MSVC 4127 warning
  bool valid = false;
  VBK_ASSERT(valid && "should not reach here");
}

}  // namespace altintegration
