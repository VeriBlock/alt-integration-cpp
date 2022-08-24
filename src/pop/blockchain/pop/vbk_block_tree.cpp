// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/blockchain/pop/vbk_block_tree.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "veriblock/pop/arith_uint256.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/blockchain/base_block_tree.hpp"
#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/blockchain/block_status.hpp"
#include "veriblock/pop/blockchain/blockchain_util.hpp"
#include "veriblock/pop/blockchain/blocktree.hpp"
#include "veriblock/pop/blockchain/btc_chain_params.hpp"
#include "veriblock/pop/blockchain/chain.hpp"
#include "veriblock/pop/blockchain/command_group.hpp"
#include "veriblock/pop/blockchain/payloads_index.hpp"
#include "veriblock/pop/blockchain/pop/fork_resolution.hpp"
#include "veriblock/pop/blockchain/vbk_chain_params.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/entities/endorsements.hpp"
#include "veriblock/pop/entities/merkle_tree.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"
#include "veriblock/pop/entities/vbkpoptx.hpp"
#include "veriblock/pop/entities/vtb.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/reversed_range.hpp"
#include "veriblock/pop/stateless_validation.hpp"
#include "veriblock/pop/strutil.hpp"
#include "veriblock/pop/third_party/Signals.hpp"
#include "veriblock/pop/trace.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {
struct BlockReader;
struct PayloadsStorage;

template struct BlockIndex<BtcBlock>;
template struct BlockTree<BtcBlock, BtcChainParams>;
template struct BaseBlockTree<BtcBlock>;
template struct BlockIndex<VbkBlock>;
template struct BlockTree<VbkBlock, VbkChainParams>;
template struct BaseBlockTree<VbkBlock>;

void VbkBlockTree::determineBestChain(index_t& candidate,
                                      ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_ASSERT(isBootstrapped());

  auto bestTip = getBestChain().tip();
  if ((bestTip->getHeight() - candidate.getHeight()) >
      param_->getMaxReorgBlocks()) {
    VBK_LOG_DEBUG("%s Candidate: %s is behind tip more than %d blocks",
                  block_t::name(),
                  candidate.toShortPrettyString(),
                  param_->getMaxReorgBlocks());
    return;
  }

  // POP FR must not be executed during block loading
  VBK_ASSERT(!this->isLoadingBlocks_);
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
  VBK_ASSERT(!this->isLoadingBlocks_);

  bool success = cmp_.setState(to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }

  VBK_ASSERT(appliedBlockCount == activeChain_.blocksCount());
  return success;
}

void VbkBlockTree::overrideTip(index_t& to) {
  VBK_TRACE_ZONE_SCOPED;
  base::overrideTip(to);
  VBK_ASSERT_MSG(to.isValid(BLOCK_CAN_BE_APPLIED),
                 "the active chain tip(%s) must be fully valid",
                 to.toPrettyString());
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

  auto containingHash = index.getHash();
  for (const auto& pid : pids) {
    auto& vtbids = index.getPayloadIds<VTB>();
    auto it = std::find(vtbids.begin(), vtbids.end(), pid);
    // using an assert because throwing breaks atomicity
    // if there are missing pids
    VBK_ASSERT(it != vtbids.end() && "could not find the payload to remove");

    // removing a payload cannot alter the block validity as addPayloads adds
    // only valid payloads
    VBK_ASSERT_MSG(!index.hasFlags(BLOCK_FAILED_POP),
                   "block %s unexpectedly has BLOCK_FAILED_POP set",
                   index.toPrettyString());

    index.removePayloadId<VTB>(pid);
    payloadsIndex_.remove(pid.asVector(), containingHash);
  }

  updateTips();
}

void VbkBlockTree::unsafelyRemovePayload(const hash_t& hash, const pid_t& pid) {
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
  payloadsIndex_.remove(pid.asVector(), index.getHash());

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
  payloadsIndex_.add(pid.asVector(), index.getHash());

  auto cmdGroup = commandGroupStore_.getCommand(index, pid, state);

  if (!cmdGroup || !cmdGroup->execute(state)) {
    VBK_LOG_DEBUG("Failed to apply payload %s to block %s: %s",
                  index.toPrettyString(),
                  pid.toHex(),
                  state.toString());

    // remove the failed payload
    index.removePayloadId<payloads_t>(pid);
    payloadsIndex_.remove(pid.asVector(), index.getHash());

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
  VBK_ASSERT(!this->isLoadingBlocks_);

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
                                    bool fast_load,
                                    ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  if (!VbkTree::loadBlockForward(index, fast_load, state)) {
    return false;  // already set
  }
  return loadBlockInner(index, fast_load, state);
}

bool VbkBlockTree::loadBlockInner(const stored_index_t& index,
                                  bool fast_load,
                                  ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_ASSERT(!this->isLoaded_);
  this->isLoadingBlocks_ = true;

  auto hash = index.header->getHash();
  auto height = index.height;

  auto* current = getBlockIndex(hash);
  VBK_ASSERT(current);

  const auto& vtbIds = current->getPayloadIds<VTB>();

  if (!fast_load && hasDuplicateIdsOf<VTB>(vtbIds, state)) {
    return false;
  }

  // recover `endorsedBy`
  if (!fast_load) {
    const auto si = param_->getEndorsementSettlementInterval();
    auto window = std::max(0, height - si);
    Chain<index_t> chain(window, current);
    if (!recoverEndorsements(*this, *current, chain, state)) {
      return state.Invalid("bad-endorsements");
    }
  } else {
    recoverEndorsementsFast(*this, *current);
  }

  if (!current->finalized) {
    payloadsIndex_.addBlock(*current);
  } else {
    finalizedPayloadsIndex_.addBlock(*current);
  }

  return true;
}

void VbkBlockTree::finalizeBlocks() {
  VBK_ASSERT(!this->isLoadingBlocks_);
  VBK_ASSERT(appliedBlockCount == activeChain_.blocksCount());

  const auto* btctip = btc().getBestChain().tip();
  VBK_ASSERT(btctip != nullptr);
  int32_t minVbkRefHeight =
      min_or_default(btctip->getRefs(), std::numeric_limits<int32_t>::max());

  // first, finalize VBK
  base::finalizeBlocks(this->getParams().getMaxReorgBlocks(),
                       this->getParams().preserveBlocksBehindFinal(),
                       /*maxFinalizeBlockHeihht=*/minVbkRefHeight);

  // then, finalize BTC
  btc().finalizeBlocks();

  VBK_ASSERT(appliedBlockCount == activeChain_.blocksCount());
}

VbkBlockTree::VbkBlockTree(const VbkChainParams& vbkp,
                           const BtcChainParams& btcp,
                           PayloadsStorage& payloadsProvider,
                           BlockReader& blockProvider)
    : VbkTree(vbkp, blockProvider),
      cmp_(*this,
           std::make_shared<BtcTree>(btcp, blockProvider),
           vbkp,
           payloadsProvider),
      payloadsProvider_(payloadsProvider),
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

  VBK_ASSERT(appliedBlockCount == activeChain_.blocksCount());

  return true;
}

uint32_t VbkBlockTree::estimateNumberOfVTBs(
    const VbkBlockTree::index_t& index) const {
  auto vtb_ids = index.getPayloadIds<VTB>();
  if (vtb_ids.empty()) {
    return std::numeric_limits<uint32_t>::max();
  }

  auto read_vtb = [this](const VTB::id_t& id) -> VTB {
    VTB vtb;
    ValidationState state;
    bool res = this->payloadsProvider_.getVTB(id, vtb, state);
    VBK_ASSERT_MSG(res, state.toString());
    return vtb;
  };

  // read get VbkMerklePath
  std::vector<VbkMerklePath> vtb_paths;
  vtb_paths.reserve(vtb_ids.size());
  for (const auto& id : vtb_ids) {
    vtb_paths.push_back(read_vtb(id).merklePath);
  }

  uint32_t approximate_count = estimateNumberOfPopTxs(vtb_paths);

  return approximate_count - vtb_paths.size();
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
                 "Block hash and previous hash are equal = %s. A collision in "
                 "altchain hash?",
                 HexStr(block.getShortHash()));
}

}  // namespace altintegration
