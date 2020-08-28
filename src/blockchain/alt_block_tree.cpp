// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/reversed_range.hpp>
#include <veriblock/storage/block_batch_adaptor.hpp>

#include "veriblock/algorithm.hpp"
#include "veriblock/command_group_cache.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"

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
  int64_t maxTime = currentTimestamp4() + params.maxFutureBlockTime();
  if (blockTime > maxTime) {
    return state.Invalid("alt-time-too-new",
                         "ALT block timestamp too far in the future");
  }

  return true;
}

bool AltTree::bootstrap(ValidationState& state) {
  if (base::isBootstrapped()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(std::make_shared<AltBlock>(std::move(block)));
  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  auto height = index->getHeight();

  index->setFlag(BLOCK_APPLIED);
  index->setFlag(BLOCK_CAN_BE_APPLIED);
  index->setFlag(BLOCK_BOOTSTRAP);
  index->setFlag(BLOCK_HAS_PAYLOADS);
  index->setFlag(BLOCK_CONNECTED);
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

/** Find stateless and stateful duplicates
 * Reorder the container items, moving all found duplicates to the back
 * @return an iterator to the first duplicate
 */
template <typename Element, typename Container>
/*iterator*/ auto findDuplicates(BlockIndex<AltBlock>& index,
                                 Container& payloads,
                                 AltTree& tree) -> decltype(payloads.end()) {
  const auto startHeight = tree.getParams().getBootstrapBlock().height;
  // don't look for duplicates in index itself
  Chain<BlockIndex<AltBlock>> chain(startHeight, index.pprev);
  std::unordered_set<std::vector<uint8_t>> ids;

  const auto& storage = tree.getPayloadsIndex();
  auto duplicates =
      std::remove_if(payloads.begin(), payloads.end(), [&](const Element& p) {
        const auto id = getIdVector(p);
        // ensure existing blocks do not contain this id
        for (const auto& hash : storage.getContainingAltBlocks(id)) {
          const auto* candidate = tree.getBlockIndex(hash);
          if (chain.contains(candidate)) {
            // duplicate in 'candidate'
            return true;
          }
        }

        // ensure ids are unique within `pop`
        bool inserted = ids.insert(id).second;
        return !inserted;
      });

  return duplicates;
}

template <typename P>
void removeDuplicates(BlockIndex<AltBlock>& index,
                      std::vector<P>& payloads,
                      AltTree& tree) {
  auto duplicates = findDuplicates<P>(index, payloads, tree);
  payloads.erase(duplicates, payloads.end());
}

template <typename I>
bool hasDuplicateIds(const std::vector<I> payloads) {
  std::unordered_set<std::vector<uint8_t>> ids;
  for (const auto& payload : payloads) {
    const auto id = getIdVector(payload);
    bool inserted = ids.insert(id).second;
    if (!inserted) {
      return true;
    }
  }
  return false;
}

template <typename P, typename T>
bool hasDuplicates(BlockIndex<AltBlock>& index,
                   std::vector<T> payloads,
                   AltTree& tree,
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

void AltTree::acceptBlock(const hash_t& block, const PopData& payloads) {
  auto* index = getBlockIndex(block);
  VBK_ASSERT_MSG(index, "cannot find block %s", HexStr(block));
  return acceptBlock(*index, payloads);
}

void AltTree::acceptBlock(index_t& index, const PopData& payloads) {
  setPayloads(index, payloads);

  if (index.pprev->hasFlags(BLOCK_CONNECTED)) {
    ValidationState dummy;
    connectBlock(index, dummy);
  };
}

void AltTree::setPayloads(index_t& index, const PopData& payloads) {
  VBK_LOG_INFO("%s add %d VBK, %d VTB, %d ATV payloads to block %s",
               block_t::name(),
               payloads.context.size(),
               payloads.vtbs.size(),
               payloads.atvs.size(),
               index.toShortPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_HAS_PAYLOADS),
                 "block %s already contains payloads",
                 index.toPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_CONNECTED),
                 "state corruption: block %s is connected",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_APPLIED),
                 "state corruption: block %s is applied",
                 index.toPrettyString());

  VBK_ASSERT_MSG(index.pprev,
                 "Adding payloads to a bootstrap block is not allowed");

  // FIXME: this check belongs to popData
  VBK_ASSERT_MSG(!hasDuplicateIds(payloads.context),
                 "block %s must not contain duplicate context VBK blocks",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!hasDuplicateIds(payloads.vtbs),
                 "block %s must not contain duplicate VBKs",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!hasDuplicateIds(payloads.atvs),
                 "block %s must not contain duplicate ATVs",
                 index.toPrettyString());

  // add payload ids to the block, update the payload index
  commitPayloadsIds<VbkBlock>(index, payloads.context, payloadsIndex_);
  commitPayloadsIds<VTB>(index, payloads.vtbs, payloadsIndex_);
  commitPayloadsIds<ATV>(index, payloads.atvs, payloadsIndex_);

  // we successfully added this block payloads
  index.setFlag(BLOCK_HAS_PAYLOADS);
}

bool AltTree::connectBlock(index_t& index, ValidationState& state) {
  VBK_ASSERT_MSG(index.hasFlags(BLOCK_HAS_PAYLOADS),
                 "block %s must have payloads added",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_CONNECTED),
                 "block %s is already connected",
                 index.toPrettyString());
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_APPLIED),
                 "state corruption: block %s is applied",
                 index.toPrettyString());

  if (getParams().isStrictAddPayloadsOrderingEnabled()) {
    VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_CONNECTED),
                   "the previous block of block %s must be connected",
                   index.toPrettyString());
    VBK_ASSERT_MSG(index.allDescendantsUnconnected(),
                   "a descendant of block %s is connected",
                   index.toPrettyString());
  }

  // partial stateful validation
  // FIXME: eventually we want to perform full stateful validation here,
  // effectively find out whether setState(index) will be successful

  // BUG: applyBlock will clear BLOCK_FAILED_POP flag since it does not check
  // for duplicates
  if (hasDuplicates<VbkBlock>(
          index, index.getPayloadIds<VbkBlock>(), *this, state) ||
      hasDuplicates<VTB>(index, index.getPayloadIds<VTB>(), *this, state) ||
      hasDuplicates<ATV>(index, index.getPayloadIds<ATV>(), *this, state)) {
    invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
  }

  index.setFlag(BLOCK_CONNECTED);
  tryAddTip(&index);

  if (index.isValid()) {
    onBlockConnected.emit(index);
  } else {
    onInvalidBlockConnected.emit(index, state);
  };

  // connect the descendants
  for (auto* successor : index.pnext) {
    if (successor->hasFlags(BLOCK_HAS_PAYLOADS)) {
      ValidationState dummy;
      connectBlock(*successor, dummy);
    }
  }

  return index.isValid();
}

// in !StrictAddPayloadsOrdering mode, payloads can be added to any block, which
// may trigger incorrect AltTree behavior in certain cases
bool AltTree::addPayloads(index_t& index,
                          PopData& payloads,
                          ValidationState& state) {
  if (getParams().isStrictAddPayloadsOrderingEnabled()) {
    // atomicity: ensure we can not just add payloads but connect the block
    VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_CONNECTED),
                   "the previous block of block %s must be connected",
                   index.toPrettyString());
  } else {
    // when StrictAddPayloadsOrdering is disabled, we allow to AddPayloads in
    // random parts of chain, so protect ourselves from adding payloads to
    // applied chain here
    bool isOnActiveChain = activeChain_.contains(&index);
    if (isOnActiveChain) {
      ValidationState dummy;
      bool success = setState(*index.pprev, dummy);
      VBK_ASSERT(success);
    }
  }

  // NOTE: we should be able to add payloads to an invalid block
  // this check is for backwards-compatibility only
  if (!index.isValid()) {
    return state.Invalid(block_t::name() + "-bad-chain",
                         "Containing block has been marked as invalid");
  }

  setPayloads(index, payloads);
  return connectBlock(index, state);
}

bool AltTree::acceptBlockHeader(const AltBlock& block, ValidationState& state) {
  if (getBlockIndex(block.getHash()) != nullptr) {
    // duplicate
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

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& tip) {
  VBK_ASSERT(isBootstrapped() && "not bootstrapped");

  auto* index = getBlockIndex(tip);
  VBK_ASSERT_MSG(index, "can not find block %s", HexStr(tip));
  VBK_ASSERT_MSG(index == activeChain_.tip(),
                 "AltTree is at unexpected state: Tip=%s ExpectedTip=%s",
                 activeChain_.tip()->toPrettyString(),
                 index->toPrettyString());
  if (getParams().isStrictAddPayloadsOrderingEnabled()) {
    VBK_ASSERT_MSG(index->hasFlags(BLOCK_CONNECTED),
                   "Block %s is not connected",
                   index->toPrettyString());
    VBK_ASSERT_MSG(index->hasFlags(BLOCK_HAS_PAYLOADS),
                   "state corruption: Block %s has no payloads",
                   index->toPrettyString());
  }

  auto* endorsedBlock = index->getAncestorBlocksBehind(
      alt_config_->getEndorsementSettlementInterval());
  if (endorsedBlock == nullptr) {
    // not enough blocks for payout
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(vbk(), *endorsedBlock);
  auto ret = rewards_.calculatePayouts(vbk(), *endorsedBlock, popDifficulty);
  VBK_LOG_DEBUG("Pop Difficulty=%s for block %s, paying to %d addresses",
                popDifficulty.toPrettyString(),
                index->toShortPrettyString(),
                ret.size());
  return ret;
}

std::string AltTree::toPrettyString(size_t level) const {
  std::string pad(level, ' ');
  return fmt::sprintf("%sAltTree{blocks=%llu\n%s\n%s\n%s}",
                      pad,
                      base::blocks_.size(),
                      base::toPrettyString(level + 2),
                      cmp_.toPrettyString(level + 2),
                      pad);
}

void AltTree::determineBestChain(index_t& candidate, ValidationState&) {
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

int AltTree::comparePopScore(const AltBlock::hash_t& A,
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

  if (getParams().isStrictAddPayloadsOrderingEnabled()) {
    VBK_ASSERT_MSG(left->hasFlags(BLOCK_CONNECTED), "A is not connected");
    VBK_ASSERT_MSG(left->hasFlags(BLOCK_HAS_PAYLOADS),
                   "state corruption: A has no payloads");
    VBK_ASSERT_MSG(right->hasFlags(BLOCK_CONNECTED), "B is not connected");
    VBK_ASSERT_MSG(right->hasFlags(BLOCK_HAS_PAYLOADS),
                   "state corruption: B has no payloads");
  }

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

template <typename Pop, typename Tree, typename Index>
static void clearSideEffects(Tree& tree, Index& index, PayloadsIndex& storage) {
  auto containingHash = index.getHash();
  auto& payloadIds = index.template getPayloadIds<Pop>();
  for (const auto& pid : payloadIds) {
    if (!storage.getValidity(containingHash, pid)) {
      tree.revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
      storage.setValidity(containingHash, pid, true);  // cleanup validity
    }

    storage.removeAltPayloadIndex(containingHash, pid.asVector());
  }
}

void AltTree::removeAllPayloads(index_t& index) {
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
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_APPLIED), "block is applied");

  VBK_ASSERT_MSG(index.allDescendantsUnconnected(),
                 "can remove payloads only from connected leaves");

  if (index.hasPayloads()) {
    clearSideEffects<VbkBlock>(*this, index, payloadsIndex_);
    clearSideEffects<VTB>(*this, index, payloadsIndex_);
    clearSideEffects<ATV>(*this, index, payloadsIndex_);
    index.clearPayloads();
  }

  VBK_ASSERT(!index.hasPayloads());

  index.unsetFlag(BLOCK_HAS_PAYLOADS);
  index.unsetFlag(BLOCK_CONNECTED);

  // the current block is no longer a tip
  tips_.erase(&index);
  // the previous block might have become a tip
  tryAddTip(index.pprev);
}

template <typename Payloads, typename BlockIndex>
void removePayloadsIfInvalid(std::vector<Payloads>& p,
                             PayloadsIndex& storage,
                             BlockIndex& index) {
  auto containing = index.getHash();
  auto it = std::remove_if(p.begin(), p.end(), [&](const Payloads& payloads) {
    auto pid = payloads.getId().asVector();
    bool isValid = storage.getValidity(containing, pid);
    // reset validity for this block, as this payload is in temporary block
    if (!isValid) {
      storage.setValidity(containing, pid, true);
    }
    return !isValid;
  });
  p.erase(it, p.end());
}

void addPayloadsContinueOnInvalid(AltTree& tree,
                                  AltTree::index_t& index,
                                  PopData& payloads,
                                  ValidationState& state) {
  // filter out statefully duplicate payloads
  removeDuplicates<VbkBlock>(index, payloads.context, tree);
  removeDuplicates<VTB>(index, payloads.vtbs, tree);
  removeDuplicates<ATV>(index, payloads.atvs, tree);

  bool success = tree.addPayloads(index, payloads, state);
  VBK_ASSERT(success);
}

void AltTree::filterInvalidPayloads(PopData& pop) {
  // return early
  if (pop.empty()) {
    return;
  }

  VBK_LOG_INFO("Called with VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // first, create tmp alt block
  AltBlock tmp;
  ValidationState state;
  {
    auto& tip = *getBestChain().tip();
    tmp.hash = std::vector<uint8_t>(32, 2);
    tmp.previousBlock = tip.getHash();
    tmp.timestamp = tip.getBlockTime() + 1;
    tmp.height = tip.getHeight() + 1;
    bool ret = acceptBlockHeader(tmp, state);
    VBK_ASSERT(ret);
  }

  auto* tmpindex = getBlockIndex(tmp.getHash());
  VBK_ASSERT(tmpindex != nullptr);

  addPayloadsContinueOnInvalid(*this, *tmpindex, pop, state);

  // setState in 'continueOnInvalid' mode
  setTipContinueOnInvalid(*tmpindex);

  removePayloadsIfInvalid(pop.atvs, payloadsIndex_, *tmpindex);
  removePayloadsIfInvalid(pop.vtbs, payloadsIndex_, *tmpindex);
  removePayloadsIfInvalid(pop.context, payloadsIndex_, *tmpindex);

  VBK_LOG_INFO("After filter VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // at this point `pop` contains only valid payloads

  this->removeSubtree(*tmpindex);
}

bool AltTree::addPayloads(const hash_t& block,
                          const PopData& popData,
                          ValidationState& state) {
  auto copy = popData;
  auto* index = getBlockIndex(block);
  VBK_ASSERT_MSG(index, "can't find block %s", HexStr(block));
  return addPayloads(*index, copy, state);
}

bool AltTree::setState(index_t& to, ValidationState& state) {
  if (getParams().isStrictAddPayloadsOrderingEnabled()) {
    VBK_ASSERT_MSG(to.hasFlags(BLOCK_CONNECTED),
                   "setState(%s) is called, but block has no BLOCK_CONNECTED",
                   to.toPrettyString());
    VBK_ASSERT_MSG(to.hasFlags(BLOCK_HAS_PAYLOADS),
                   "state corruption: setState(%s) is called, but block has no "
                   "BLOCK_HAS_PAYLOADS",
                   to.toPrettyString());
  }

  bool success = cmp_.setState(*this, to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }
  return success;
}

void AltTree::overrideTip(index_t& to) {
  VBK_LOG_DEBUG("ALT=\"%s\", VBK=\"%s\", BTC=\"%s\"",
                to.toShortPrettyString(),
                (vbk().getBestChain().tip()
                     ? vbk().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"),
                (btc().getBestChain().tip()
                     ? btc().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"));
  activeChain_.setTip(&to);
  tryAddTip(&to);
}

void AltTree::setTipContinueOnInvalid(AltTree::index_t& to) {
  ValidationState dummy;
  bool success = cmp_.setState(*this, to, dummy, /*continueOnInvalid=*/true);
  VBK_ASSERT(success);
  overrideTip(to);
}

bool AltTree::loadBlock(const AltTree::index_t& index, ValidationState& state) {
  if (!base::loadBlock(index, state)) {
    return false;  // already set
  }

  // load endorsements
  auto containingHash = index.getHash();
  auto* current = getBlockIndex(containingHash);
  VBK_ASSERT(current);

  auto vbkblocks = current->getPayloadIds<VbkBlock>();
  auto vtbs = current->getPayloadIds<VTB>();
  auto atvs = current->getPayloadIds<ATV>();
  if (hasDuplicates<VbkBlock>(*current, vbkblocks, *this, state) ||
      hasDuplicates<VTB>(*current, vtbs, *this, state) ||
      hasDuplicates<ATV>(*current, atvs, *this, state)) {
    return false;
  }

  // recover `endorsedBy` and `blockOfProofEndorsements`
  auto window = std::max(
      0, index.getHeight() - getParams().getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsements(*this, chain, *current, state)) {
    return state.Invalid("bad-endorsements");
  }

  payloadsIndex_.addBlockToIndex(*current);

  return true;
}

AltTree::AltTree(const AltTree::alt_config_t& alt_config,
                 const AltTree::vbk_config_t& vbk_config,
                 const AltTree::btc_config_t& btc_config,
                 PayloadsProvider& payloadsProvider)
    : alt_config_(&alt_config),
      vbk_config_(&vbk_config),
      btc_config_(&btc_config),
      cmp_(std::make_shared<VbkBlockTree>(
               vbk_config, btc_config, payloadsProvider, payloadsIndex_),
           alt_config,
           payloadsProvider,
           payloadsIndex_),
      rewards_(alt_config),
      payloadsProvider_(payloadsProvider) {}

void AltTree::removeSubtree(AltTree::index_t& toRemove) {
  payloadsIndex_.removePayloadsIndex(toRemove);
  base::removeSubtree(toRemove);
}

bool AltTree::loadTip(const AltTree::hash_t& hash, ValidationState& state) {
  if (!base::loadTip(hash, state)) {
    return false;
  }

  auto* tip = activeChain_.tip();
  VBK_ASSERT(tip);
  while (tip) {
    tip->setFlag(BLOCK_APPLIED);
    tip->setFlag(BLOCK_CAN_BE_APPLIED);
    tip = tip->pprev;
  }

  return true;
}

void AltTree::removePayloads(const AltTree::hash_t& hash) {
  auto* index = getBlockIndex(hash);
  VBK_ASSERT_MSG(index, "does not contain block %s", HexStr(hash));
  return removeAllPayloads(*index);
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
