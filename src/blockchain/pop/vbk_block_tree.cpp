// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/reversed_range.hpp>

namespace altintegration {

template struct BlockIndex<BtcBlock>;
template struct BlockTree<BtcBlock, BtcChainParams>;
template struct BaseBlockTree<BtcBlock>;
template struct BlockIndex<VbkBlock>;
template struct BlockTree<VbkBlock, VbkChainParams>;
template struct BaseBlockTree<VbkBlock>;

void VbkBlockTree::determineBestChain(index_t& candidate,
                                      ValidationState& state) {
  auto bestTip = getBestChain().tip();
  VBK_ASSERT(bestTip != nullptr && "must be bootstrapped");

  if (bestTip->getHeight() >
      candidate.getHeight() + param_->getMaxReorgBlocks()) {
    VBK_LOG_DEBUG("%s Candidate is behind tip more than %d blocks",
                  block_t::name(),
                  candidate.toShortPrettyString(),
                  param_->getMaxReorgBlocks());
    return;
  }

  int result = cmp_.comparePopScore(*this, candidate, state);
  // the pop state is already set to the best of the two chains
  if (result == 0) {
    VBK_LOG_DEBUG("Pop scores are equal");
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(candidate, state);
  } else if (result < 0) {
    VBK_LOG_DEBUG("Candidate chain won");
    // the other chain won!
    // setState(candidate) has been already done, so only update the tip
    this->overrideTip(candidate);
  } else {
    // the current chain is better
    VBK_LOG_DEBUG("Active chain won");
  }
}

bool VbkBlockTree::setState(index_t& to, ValidationState& state) {
  bool success = cmp_.setState(*this, to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }
  return success;
}

void VbkBlockTree::overrideTip(index_t& to) {
  base::overrideTip(to);
  VBK_ASSERT_MSG(to.hasFlags(BLOCK_CAN_BE_APPLIED),
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
  if (!index) {
    throw std::logic_error("removePayloads is called on unknown VBK block: " +
                           hash.toHex());
  }

  return removePayloads(*index, pids);
}

void VbkBlockTree::removePayloads(index_t& index,
                                  const std::vector<pid_t>& pids) {
  VBK_LOG_DEBUG(
      "remove %d payloads from %s", pids.size(), index.toPrettyString());

  // we do not allow adding payloads to the genesis block
  VBK_ASSERT(index.pprev && "can not remove payloads from the genesis block");

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
    // if there are multiple pids
    VBK_ASSERT(it != vtbids.end() && "could not find the payload to remove");

    if (!payloadsIndex_.getValidity(containingHash, pid)) {
      revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
    }

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
  VBK_LOG_DEBUG("unsafely removing %s payload from %s",
                pid.toPrettyString(),
                index.toPrettyString());

  auto containingHash = index.getHash();
  auto& vtbids = index.getPayloadIds<VTB>();
  auto vtbid_it = std::find(vtbids.begin(), vtbids.end(), pid);
  VBK_ASSERT(vtbid_it != vtbids.end() &&
             "state corruption: the block does not contain the payload");

  // removing an invalid payload might render the block valid
  if (!payloadsIndex_.getValidity(containingHash, pid)) {
    revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
  }

  bool isApplied = activeChain_.contains(&index);

  if (isApplied) {
    ValidationState dummy;
    std::vector<CommandGroup> cmdGroups;
    bool ret = payloadsProvider_.getCommands(*this, index, cmdGroups, dummy);
    VBK_ASSERT_MSG(ret,
                   "failed to load commands from block=%s, reason=%s",
                   index.toPrettyString(),
                   dummy.toString());
    auto group_it = std::find_if(
        cmdGroups.begin(), cmdGroups.end(), [&](CommandGroup& group) {
          return group.id == pid;
        });

    VBK_ASSERT(group_it != cmdGroups.end() &&
               "state corruption: could not find the supposedly applied "
               "command group");

    VBK_LOG_DEBUG("Unapplying payload %s in block %s",
                  HexStr(pid),
                  index.toShortPrettyString());

    group_it->unExecute();
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
  auto& tx = vtb.transaction;

  auto& firstBlock = tx.blockOfProofContext.size() > 0
                         ? tx.blockOfProofContext[0]
                         : tx.blockOfProof;

  auto connectingHash = firstBlock.previousBlock != ArithUint256()
                            ? firstBlock.previousBlock
                            : firstBlock.getHash();

  auto* connectingIndex = btc().getBlockIndex(connectingHash);
  if (!connectingIndex) {
    VBK_LOG_DEBUG("Could not find block %s that payload %s needs to connect to",
                  connectingHash.toHex(),
                  vtb.toPrettyString());
    return state.Invalid("vtb-btc-context-unknown-previous-block",
                         "Can not find the BTC block referenced by the first "
                         "block of the VTB context: " +
                             connectingHash.toHex());
  }

  bool isValid = std::any_of(connectingIndex->getRefs().begin(),
                             connectingIndex->getRefs().end(),
                             [&](BtcTree::index_t::ref_height_t height) {
                               return height <= vtb.containingBlock.height;
                             });

  return isValid
             ? true
             : state.Invalid("vtb-btc-context-block-referenced-too-early",
                             "The BTC block referenced by the first block of "
                             "the VTB context is added "
                             "by blocks that follow the containing block: " +
                                 connectingHash.toHex());
}

bool VbkBlockTree::addPayloadToAppliedBlock(index_t& index,
                                            const payloads_t& payload,
                                            ValidationState& state) {
  VBK_ASSERT(index.hasFlags(BLOCK_APPLIED));

  auto pid = payload.getId();
  VBK_LOG_DEBUG("Adding and applying payload %s in block %s",
                pid.toHex(),
                index.toShortPrettyString());

  if (!validateBTCContext(payload, state)) {
    return state.Invalid(
        block_t::name() + "-btc-context-does-not-connect",
        fmt::sprintf("payload %s we attempted to add to block %s has "
                     "the BTC context that does not connect to the BTC tree",
                     payload.toPrettyString(),
                     index.toPrettyString()));
  }

  index.insertPayloadId<payloads_t>(pid);
  payloadsIndex_.addVbkPayloadIndex(index.getHash(), pid.asVector());

  // load commands from block
  std::vector<CommandGroup> cmdGroups;
  bool ret = payloadsProvider_.getCommands(*this, index, cmdGroups, state);
  VBK_ASSERT_MSG(ret,
                 "failed to load commands from block=%s, reason=%s",
                 index.toPrettyString(),
                 state.toString());
  auto group_it = std::find_if(
      cmdGroups.begin(), cmdGroups.end(), [&](CommandGroup& group) {
        return group.id == pid;
      });

  VBK_ASSERT(group_it != cmdGroups.end() &&
             "state corruption: could not find the command group that "
             "corresponds to the payload we have just added");

  if (!group_it->execute(state)) {
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
  VBK_LOG_DEBUG("%s add %d payloads to block %s",
                block_t::name(),
                payloads.size(),
                HexStr(hash));
  if (payloads.empty()) {
    return true;
  }

  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    return state.Invalid(block_t::name() + "-bad-containing",
                         "Can not find VTB containing block: " + hash.toHex());
  }

  if (!index->pprev) {
    return state.Invalid(block_t::name() + "-bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  // TODO: once we plug the validation hole, we want this to be an assert
  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return state.Invalid(
        block_t::name() + "-bad-chain",
        fmt::sprintf("Containing block=%s is added on top of invalid chain",
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
          fmt::sprintf(
              "Containing block=%s could not be applied due to being invalid",
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
          fmt::sprintf("Attempted to add invalid payload %s to block %s",
                       pid.toHex(),
                       index->toPrettyString()));
    }

    appliedPayloads.push_back(pid);
  }

  // don't defer fork resolution in the acceptBlock+addPayloads flow until the
  // validation hole is plugged
  doUpdateAffectedTips(*index, state);

  // compare to the original best chain
  if (!isOnActiveChain) {
    doUpdateAffectedTips(*tip, state);
  }

  return true;
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  return fmt::sprintf(
      "%s\n%s", VbkTree::toPrettyString(level), cmp_.toPrettyString(level + 2));
}

bool VbkBlockTree::loadBlock(const VbkBlockTree::index_t& index,
                             ValidationState& state) {
  if (!VbkTree::loadBlock(index, state)) {
    return false;  // already set
  }

  auto* current = getBlockIndex(index.getHash());
  VBK_ASSERT(current);

  // TODO: check for duplicates

  // recover `endorsedBy`
  auto window = std::max(
      0, index.getHeight() - param_->getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsements(*this, chain, *current, state)) {
    return state.Invalid("bad-endorsements");
  }

  payloadsIndex_.addBlockToIndex(*current);

  return true;
}

void VbkBlockTree::removeSubtree(VbkBlockTree::index_t& toRemove) {
  payloadsIndex_.removePayloadsIndex(toRemove);
  BaseBlockTree::removeSubtree(toRemove);
}

VbkBlockTree::VbkBlockTree(const VbkChainParams& vbkp,
                           const BtcChainParams& btcp,
                           PayloadsProvider& storagePayloads,
                           PayloadsIndex& payloadsIndex)
    : VbkTree(vbkp),
      cmp_(std::make_shared<BtcTree>(btcp),
           vbkp,
           storagePayloads,
           payloadsIndex),
      payloadsProvider_(storagePayloads),
      payloadsIndex_(payloadsIndex) {}

bool VbkBlockTree::loadTip(const Blob<24>& hash, ValidationState& state) {
  if (!base::loadTip(hash, state)) {
    return false;
  }

  auto* tip = activeChain_.tip();
  VBK_ASSERT(tip);
  appliedBlockCount = 0;
  while (tip) {
    tip->setFlag(BLOCK_APPLIED);
    ++appliedBlockCount;
    tip->setFlag(BLOCK_CAN_BE_APPLIED);
    tip = tip->pprev;
  }

  return true;
}

bool VbkBlockTree::areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  return (vtb1.transaction.bitcoinTransaction ==
          vtb2.transaction.bitcoinTransaction) &&
         (vtb1.transaction.blockOfProof == vtb2.transaction.blockOfProof);
}

bool VbkBlockTree::areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  bool is_on_the_same_btc_chain = btc().areOnSameChain(
      vtb1.transaction.blockOfProof, vtb2.transaction.blockOfProof);

  bool is_on_the_same_vbk_chain = areOnSameChain(
      vtb1.transaction.publishedBlock, vtb2.transaction.publishedBlock);

  bool are_the_same_keystone_period =
      areOnSameKeystoneInterval(vtb1.transaction.publishedBlock.height,
                                vtb2.transaction.publishedBlock.height,
                                getParams().getKeystoneInterval());

  return ((vtb1.transaction.publishedBlock ==
           vtb2.transaction.publishedBlock) &&
          is_on_the_same_btc_chain) ||
         (is_on_the_same_vbk_chain && are_the_same_keystone_period &&
          is_on_the_same_btc_chain);
}

int VbkBlockTree::weaklyCompare(const VTB& vtb1, const VTB& vtb2) {
  VBK_ASSERT_MSG(areWeaklyEquivalent(vtb1, vtb2),
                 "vtbs should be weakly equivalent");

  auto* blockOfProof1 =
      btc().getBlockIndex(vtb1.transaction.blockOfProof.getHash());
  auto* blockOfProof2 =
      btc().getBlockIndex(vtb2.transaction.blockOfProof.getHash());

  VBK_ASSERT_MSG(blockOfProof1,
                 "unknown block %s",
                 vtb1.transaction.blockOfProof.toPrettyString());
  VBK_ASSERT_MSG(blockOfProof2,
                 "unknown block %s",
                 vtb2.transaction.blockOfProof.toPrettyString());

  if (blockOfProof1->getHeight() > blockOfProof2->getHeight()) {
    return -1;
  }

  if (blockOfProof1->getHeight() < blockOfProof2->getHeight()) {
    return 1;
  }

  if (vtb1.transaction.publishedBlock.height >
      vtb2.transaction.publishedBlock.height) {
    return -1;
  }

  if (vtb1.transaction.publishedBlock.height <
      vtb2.transaction.publishedBlock.height) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() >
      vtb2.transaction.blockOfProofContext.size()) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() <
      vtb2.transaction.blockOfProofContext.size()) {
    return -1;
  }

  VBK_ASSERT_MSG(
      areStronglyEquivalent(vtb1, vtb2),
      "if we can not define the best vtb they should be strongly equivalent");
  return 0;
}

template <>
void assertBlockCanBeRemoved(const BlockIndex<BtcBlock>& index) {
  VBK_ASSERT_MSG(index.blockOfProofEndorsements.empty(),
                 "blockOfProof has %d pointers to endorsements, they will be "
                 "lost",
                 index.blockOfProofEndorsements.size());
}

template <>
void assertBlockCanBeRemoved(const BlockIndex<VbkBlock>& index) {
  VBK_ASSERT_MSG(index.blockOfProofEndorsements.empty(),
                 "blockOfProof has %d pointers to endorsements, they will be "
                 "lost",
                 index.blockOfProofEndorsements.size());

  VBK_ASSERT_MSG(index.endorsedBy.empty(),
                 "endorsedBy has %d pointers to endorsements, they will be "
                 "lost",
                 index.endorsedBy.size());
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
