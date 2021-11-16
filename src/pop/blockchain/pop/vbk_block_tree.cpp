// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/blockchain/commands/commands.hpp>
#include <veriblock/pop/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/pop/finalizer.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/reversed_range.hpp>
#include <veriblock/pop/trace.hpp>

namespace altintegration {

template struct BlockIndex<BtcBlock>;
template struct BlockTree<BtcBlock, BtcChainParams>;
template struct BaseBlockTree<BtcBlock>;
template struct BlockIndex<VbkBlock>;
template struct BlockTree<VbkBlock, VbkChainParams>;
template struct BaseBlockTree<VbkBlock>;

void VbkBlockTree::determineBestChain(index_t& candidate,
                                      ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  auto bestTip = getBestChain().tip();
  VBK_ASSERT(bestTip != nullptr && "must be bootstrapped");

  if (bestTip->getHeight() >
      candidate.getHeight() + param_->getMaxReorgBlocks()) {
    VBK_LOG_DEBUG("%s Candidate: %s is behind tip more than %d blocks",
                  block_t::name(),
                  candidate.toShortPrettyString(),
                  param_->getMaxReorgBlocks());
    return;
  }

  auto p = cmp_.comparePopScore(candidate, state);
  auto result = p.first;
  auto reason = p.second;
  // the pop state is already set to the best of the two chains
  if (result == 0) {
    VBK_LOG_DEBUG("Pop scores are equal. Reason: %s.",
                  popFrOutcomeToString(reason, state));
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(candidate, state);
  } else if (result < 0) {
    VBK_LOG_DEBUG("Candidate chain won. Reason: %s.",
                  popFrOutcomeToString(reason, state));
    // the other chain won!
    // setState(candidate) has been already done, so only update the tip
    this->overrideTip(candidate);
  } else {
    // the current chain is better
    VBK_LOG_DEBUG("Active chain won. Reason: %s.",
                  popFrOutcomeToString(reason, state));
  }
}

bool VbkBlockTree::setState(index_t& to, ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  bool success = cmp_.setState(to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }
  return success;
}

void VbkBlockTree::overrideTip(index_t& to) {
  VBK_TRACE_ZONE_SCOPED;
  base::overrideTip(to);
  VBK_ASSERT_MSG(to.isValid(BLOCK_CAN_BE_APPLIED),
                 "the active chain tip(%s) must be fully valid",
                 to.toPrettyString());
}

void VbkBlockTree::removePayloads(const block_t& block,
                                  const std::vector<pid_t>& pids) {
  return removePayloads(block.getHash(), pids);
}

void VbkBlockTree::removePayloads(const hash_t& hash,
                                  const std::vector<pid_t>& pids) {
  auto index = VbkTree::getBlockIndex(hash);
  VBK_ASSERT_MSG(index, "block %s must exist", HexStr(hash));
  return removePayloads(*index, pids);
}

void VbkBlockTree::removePayloads(index_t& index,
                                  const std::vector<pid_t>& pids) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_LOG_DEBUG(
      "remove %d payloads from %s", pids.size(), index.toPrettyString());

  // we do not allow adding payloads to the root block
  VBK_ASSERT_MSG(!index.isRoot(),
                 "can not remove payloads from the root block");

  if (pids.empty()) {
    return;
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool success = setState(*index.pprev, dummy);
    VBK_ASSERT(success);
  }

  for (const auto& pid : pids) {
    auto& vtbids = index.getPayloadIds<VTB>();
    auto it = std::find(vtbids.begin(), vtbids.end(), pid);
    // using an assert because throwing breaks atomicity
    // if there are multiple pids
    VBK_ASSERT(it != vtbids.end() && "could not find the payload to remove");

    // removing a payload cannot alter the block validity as addPayloads adds
    // only valid payloads
    VBK_ASSERT_MSG(!index.hasFlags(BLOCK_FAILED_POP),
                   "block %s unexpectedly has BLOCK_FAILED_POP set",
                   index.toPrettyString());

    index.removePayloadId<VTB>(pid);
    payloadsIndex_.removeVbkPayloadIndex(index.getHash(), pid.asVector());
  }

  updateTips();
}

void VbkBlockTree::unsafelyRemovePayload(const block_t& block,
                                         const pid_t& pid) {
  return unsafelyRemovePayload(block.getHash(), pid);
}

void VbkBlockTree::unsafelyRemovePayload(const Blob<24>& hash,
                                         const pid_t& pid) {
  auto index = VbkTree::getBlockIndex(hash);
  VBK_ASSERT(index != nullptr &&
             "state corruption: the containing block is not found");

  return unsafelyRemovePayload(*index, pid);
}

void VbkBlockTree::unsafelyRemovePayload(index_t& index,
                                         const pid_t& pid,
                                         bool shouldDetermineBestChain) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_LOG_DEBUG("unsafely removing %s payload from %s",
                pid.toPrettyString(),
                index.toPrettyString());

  auto& vtbids = index.getPayloadIds<VTB>();
  auto vtbid_it = std::find(vtbids.begin(), vtbids.end(), pid);
  VBK_ASSERT_MSG(vtbid_it != vtbids.end(),
                 "state corruption: the block does not contain the payload");

  // removing a payload cannot alter the block validity as addPayloads adds only
  // valid payloads
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_FAILED_POP),
                 "block %s unexpectedly has BLOCK_FAILED_POP set",
                 index.toPrettyString());

  bool isApplied = activeChain_.contains(&index);
  if (isApplied) {
    ValidationState state;
    auto cmdGroup = commandGroupStore_.getCommand(index, pid, state);

    VBK_ASSERT_MSG(cmdGroup,
                   "state corruption: could not pre-validate a supposedly "
                   "applied command group: %s",
                   state.toString());

    VBK_LOG_DEBUG("Unapplying payload %s in block %s",
                  HexStr(pid),
                  index.toShortPrettyString());

    cmdGroup->unExecute();
  }

  index.removePayloadId<VTB>(pid);
  payloadsIndex_.removeVbkPayloadIndex(index.getHash(), pid.asVector());

  if (shouldDetermineBestChain) {
    updateTips();
  }
}

// temporal validation: the connecting BTC block must be added in the containing
// or earlier blocks
bool VbkBlockTree::validateBTCContext(const VbkBlockTree::payloads_t& vtb,
                                      ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  auto& tx = vtb.transaction;

  auto& firstBlock = !tx.blockOfProofContext.empty()
                         ? tx.blockOfProofContext.front()
                         : tx.blockOfProof;

  // if 'firstBlock' is not genesis block, use 'previousBlock' as connectingHash
  auto connectingHash = firstBlock.getPreviousBlock() != uint256()
                            ? firstBlock.getPreviousBlock()
                            : firstBlock.getHash();

  auto* connectingIndex = btc().getBlockIndex(connectingHash);
  if (connectingIndex == nullptr) {
    return state.Invalid("bad-prev-block",
                         "Can not find the BTC block referenced by the first "
                         "block of the VTB context");
  }

  bool isValid = std::any_of(connectingIndex->getRefs().begin(),
                             connectingIndex->getRefs().end(),
                             [&](BtcTree::index_t::ref_height_t height) {
                               return height <= vtb.containingBlock.getHeight();
                             });

  if (isValid) return true;
  return state.Invalid("block-referenced-too-early");
}

bool VbkBlockTree::addPayloadToAppliedBlock(index_t& index,
                                            const payloads_t& payload,
                                            ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  // in this method we allow to add duplicates because of the functionality of
  // the forkresolution algorithms.
  // look into the description alt_blockchain_test.cpp
  // duplicateVTBs_test test case

  VBK_ASSERT(index.hasFlags(BLOCK_ACTIVE));

  auto pid = payload.getId();
  VBK_LOG_DEBUG("Adding and applying payload %s in block %s",
                pid.toHex(),
                index.toShortPrettyString());

  // we compare with the previous amount of payloads because we have not add the
  // current payload into this vector
  if (index.getPayloadIds<payloads_t>().size() >= MAX_VBKPOPTX_PER_VBK_BLOCK) {
    return state.Invalid(block_t::name() + "-invalid-poptxs-amount",
                         "The amount of the pop txs which we try to add into "
                         "the block more than maximum");
  }

  if (!validateBTCContext(payload, state)) {
    VBK_LOG_DEBUG("Could not find block that payload %s needs to connect to",
                  payload.toPrettyString());
    return state.Invalid(
        block_t::name() + "-btc-context-does-not-connect",
        format("payload {} we attempted to add to block {} has "
               "the BTC context that does not connect to the BTC tree",
               payload.toPrettyString(),
               index.toPrettyString()));
  }

  index.insertPayloadId<payloads_t>(pid);
  payloadsIndex_.addVbkPayloadIndex(index.getHash(), pid.asVector());

  auto cmdGroup = commandGroupStore_.getCommand(index, pid, state);

  if (!cmdGroup || !cmdGroup->execute(state)) {
    VBK_LOG_DEBUG("Failed to apply payload %s to block %s: %s",
                  index.toPrettyString(),
                  pid.toHex(),
                  state.toString());

    // remove the failed payload
    index.removePayloadId<payloads_t>(pid);
    payloadsIndex_.removeVbkPayloadIndex(index.getHash(), pid.asVector());

    return false;
  }

  return true;
}

bool VbkBlockTree::addPayloads(const VbkBlock::hash_t& hash,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_LOG_DEBUG("%s add %d payloads to block %s",
                block_t::name(),
                payloads.size(),
                HexStr(hash));
  if (payloads.empty()) {
    return true;
  }

  auto* index = VbkTree::getBlockIndex(hash);
  if (index == nullptr) {
    return state.Invalid(block_t::name() + "-bad-containing",
                         "Can not find VTB containing block: " + hash.toHex());
  }

  // TODO: once we plug the validation hole, we want this to be an assert
  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return state.Invalid(
        block_t::name() + "-bad-chain",
        format("Containing block={} is added on top of invalid chain",
               index->toPrettyString()));
  }

  auto tip = activeChain_.tip();
  VBK_ASSERT(tip != nullptr);

  bool isOnActiveChain = activeChain_.contains(index);

  // we need the index to be applied in order to validate VTBs efficiently
  if (!isOnActiveChain) {
    bool success = setState(*index, state);
    if (!success) {
      // TODO: once we plug the validation hole, we want this to be an assert
      return state.Invalid(
          block_t::name() + "-invalid-containing-block",
          format(
              "Containing block={} could not be applied due to being invalid",
              index->toPrettyString()));
    }
  }

  // apply payloads
  std::vector<pid_t> appliedPayloads;

  for (const auto& payload : payloads) {
    auto pid = payload.getId();

    bool added = addPayloadToAppliedBlock(*index, payload, state);
    if (!added) {
      // roll back previously applied payloads
      for (const auto& pidToRemove : reverse_iterate(appliedPayloads)) {
        unsafelyRemovePayload(
            *index, pidToRemove, /*shouldDetermineBestChain =*/false);
      }

      // restore the tip
      if (!isOnActiveChain) {
        bool success = setState(*tip, state);
        VBK_ASSERT(success &&
                   "state corruption: failed to restore the best chain tip");
      }

      return state.Invalid(
          block_t::name() + "-invalid-payloads",
          format("Attempted to add invalid payload {} to block {}: {}",
                 pid.toHex(),
                 index->toPrettyString(),
                 state.GetDebugMessage()));
    }

    appliedPayloads.push_back(pid);
  }

  // don't defer fork resolution in the acceptBlockHeader+addPayloads flow until
  // the validation hole is plugged
  doUpdateAffectedTips(*index, state);

  // compare to the original best chain
  if (!isOnActiveChain) {
    doUpdateAffectedTips(*tip, state);
  }

  return true;
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  return format(
      "{}\n{}", VbkTree::toPrettyString(level), cmp_.toPrettyString(level + 2));
}

bool VbkBlockTree::loadBlockForward(const stored_index_t& index,
                                    ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  if (!VbkTree::loadBlockForward(index, state)) {
    return false;  // already set
  }
  return loadBlockInner(index, state);
}

bool VbkBlockTree::loadBlockBackward(const stored_index_t& index,
                                     ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  if (!VbkTree::loadBlockBackward(index, state)) {
    return false;
  }
  return loadBlockInner(index, state);
}

bool VbkBlockTree::loadBlockInner(const stored_index_t& index,
                                  ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  auto hash = index.header->getHash();
  auto height = index.height;

  auto* current = getBlockIndex(hash);
  VBK_ASSERT(current);

  const auto& vtbIds = current->getPayloadIds<VTB>();

  if (hasDuplicateIdsOf<VTB>(vtbIds, state)) return false;

  // recover `endorsedBy`
  const auto si = param_->getEndorsementSettlementInterval();
  auto window = std::max(0, height - si);
  Chain<index_t> chain(window, current);
  if (!recoverEndorsements(*this, chain, *current, state)) {
    return state.Invalid("bad-endorsements");
  }

  payloadsIndex_.addBlockToIndex(*current);

  return true;
}

void VbkBlockTree::removeSubtree(VbkBlockTree::index_t& toRemove) {
  VBK_TRACE_ZONE_SCOPED;
  payloadsIndex_.removePayloadsIndex(toRemove);
  BaseBlockTree::removeSubtree(toRemove);
}

bool VbkBlockTree::finalizeBlockImpl(index_t& index,
                                     int32_t preserveBlocksBehindFinal,
                                     ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  auto* bestBtcTip = btc().getBestChain().tip();
  VBK_ASSERT(bestBtcTip && "BTC tree must be bootstrapped");

  int32_t firstBlockHeight =
      bestBtcTip->getHeight() - btc().getParams().getOldBlocksWindow();
  int32_t bootstrapBlockHeight = btc().getRoot().getHeight();
  firstBlockHeight = std::max(bootstrapBlockHeight, firstBlockHeight);
  auto* finalizedIndex = btc().getBestChain()[firstBlockHeight];
  VBK_ASSERT_MSG(finalizedIndex != nullptr, "Invalid BTC tree state");
  if (!btc().finalizeBlock(*finalizedIndex, state)) {
    return state.Invalid("btctree-finalize-error");
  }
  return base::finalizeBlockImpl(index, preserveBlocksBehindFinal, state);
}

VbkBlockTree::VbkBlockTree(const VbkChainParams& vbkp,
                           const BtcChainParams& btcp,
                           PayloadsStorage& payloadsProvider,
                           BlockReader& blockProvider,
                           PayloadsIndex& payloadsIndex)
    : VbkTree(vbkp, blockProvider),
      cmp_(*this,
           std::make_shared<BtcTree>(btcp, blockProvider),
           vbkp,
           payloadsProvider,
           payloadsIndex),
      payloadsProvider_(payloadsProvider),
      payloadsIndex_(payloadsIndex),
      commandGroupStore_(*this, payloadsProvider_) {}

bool VbkBlockTree::loadTip(const hash_t& hash, ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
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

template <>
void assertBlockCanBeRemoved(const BlockIndex<BtcBlock>& index) {
  VBK_ASSERT_MSG(index.getBlockOfProofEndorsement().empty(),
                 "blockOfProof has %d pointers to endorsements, they will be "
                 "lost",
                 index.getBlockOfProofEndorsement().size());
}

template <>
void assertBlockCanBeRemoved(const BlockIndex<VbkBlock>& index) {
  VBK_ASSERT_MSG(index.getBlockOfProofEndorsement().empty(),
                 "blockOfProof has %d pointers to endorsements, they will be "
                 "lost",
                 index.getBlockOfProofEndorsement().size());

  VBK_ASSERT_MSG(index.getEndorsedBy().empty(),
                 "endorsedBy has %d pointers to endorsements, they will be "
                 "lost",
                 index.getEndorsedBy().size());
}

template <>
void assertBlockSanity(const VbkBlock& block) {
  VBK_ASSERT_MSG(block.getShortHash() != block.getPreviousBlock(),
                 "Previous block hash should NOT be equal to the current block "
                 "hash: %s. A collision in altchain hash?",
                 HexStr(block.getShortHash()));
}

template <>
void removePayloadsFromIndex(PayloadsIndex& storage,
                             BlockIndex<VbkBlock>& index,
                             const CommandGroup& cg) {
  VBK_ASSERT(cg.payload_type_name == &VTB::name());
  auto& payloads = index.template getPayloadIds<VTB>();
  auto it = std::find(payloads.rbegin(), payloads.rend(), cg.id);
  VBK_ASSERT(it != payloads.rend());
  index.removePayloadId<VTB>(cg.id);
  storage.removeVbkPayloadIndex(index.getHash(), cg.id);
}

}  // namespace altintegration
