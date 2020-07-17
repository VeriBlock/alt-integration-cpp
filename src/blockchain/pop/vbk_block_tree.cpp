// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/context.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/reversed_range.hpp>

namespace altintegration {

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
  // pop state is already at "best chain"
  if (result == 0) {
    VBK_LOG_DEBUG("Pop scores are equal");
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(candidate, state);
  } else if (result < 0) {
    VBK_LOG_DEBUG("Candidate chain won");
    // other chain won! we already set pop state, so only update tip
    this->overrideTip(candidate);
  } else {
    VBK_LOG_DEBUG("Active chain won");
    // current chain is better
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

void VbkBlockTree::removePayloads(const block_t& block,
                                  const std::vector<pid_t>& pids) {
  return removePayloads(block.getHash(), pids);
}

void VbkBlockTree::removePayloads(const Blob<24>& hash,
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

    if (!storage_.getValidity(containingHash, pid)) {
      revalidateSubtree(index, BLOCK_FAILED_POP, false);
    }

    index.removePayloadId<VTB>(pid);
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
  if (!storage_.getValidity(containingHash, pid)) {
    revalidateSubtree(index, BLOCK_FAILED_POP, false);
  }

  bool isApplied = activeChain_.contains(&index);

  if (isApplied) {
    auto cmdGroups = storage_.loadCommands<VbkBlockTree>(index, *this);

    auto group_it = std::find_if(
        cmdGroups.begin(), cmdGroups.end(), [&](CommandGroup& group) {
          return group.id == pid;
        });

    VBK_ASSERT(group_it != cmdGroups.end() &&
               "state corruption: could not find the supposedly applied "
               "command group");
    auto group = *group_it;

    VBK_LOG_DEBUG("Unapplying payload %s in block %s",
                  HexStr(group.id),
                  index.toShortPrettyString());

    for (auto& cmd : reverse_iterate(group.commands)) {
      cmd->UnExecute();
    }
  }

  index.removePayloadId<VTB>(pid);

  if (shouldDetermineBestChain) {
    updateTips();
  }
}

bool VbkBlockTree::validateBTCContext(const VbkBlockTree::payloads_t& vtb,
                                      ValidationState& state) {
  auto& tx = vtb.transaction;

  auto& firstBlock = tx.blockOfProofContext.size() > 0
                         ? tx.blockOfProofContext[0]
                         : tx.blockOfProof;

  auto& connectingHash = firstBlock.previousBlock;

  auto* connectingIndex = btc().getBlockIndex(connectingHash);
  if (!connectingIndex) {
    return state.Invalid("vtb-btc-context-unknown-previous-block",
                         "Can not find the BTC block referenced by the first "
                         "block of the VTB context: " +
                             connectingHash.toHex());
  }

  bool isValid = std::any_of(connectingIndex->refs.begin(),
                             connectingIndex->refs.end(),
                             [&](BtcTree::index_t::ref_height_t height) {
                               return height <= vtb.containingBlock.height;
                             });

  if (isValid) {
    return true;
  }

  return state.Invalid(
      "vtb-btc-context-block-referenced-too-early",
      "The BTC block referenced by the first block of the VTB context is added "
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

  index.vtbids.push_back(pid);
  storage_.addVbkPayloadIndex(index->getHash(), pid.asVector());
  storagePayloads_.savePayloads(payload);

  auto cmdGroups = storagePayloads_.loadCommands<VbkBlockTree>(index, *this);

  auto group_it = std::find_if(
      cmdGroups.begin(), cmdGroups.end(), [&](CommandGroup& group) {
        return group.id == pid;
      });

  VBK_ASSERT(group_it != cmdGroups.end() &&
             "state corruption: could not find the command group that "
             "corresponds to the payload we have just added");
  auto group = *group_it;

  std::vector<CommandPtr> executed;
  for (auto& cmd : group.commands) {
    if (cmd->Execute(state)) {
      executed.push_back(cmd);

    } else {
      VBK_LOG_DEBUG("Failed to apply payload %s to block %s: %s",
                    index.toPrettyString(),
                    pid.toHex(),
                    state.toString());

      // rollback the partially executed commandGroup
      for (auto& c : reverse_iterate(executed)) {
        c->UnExecute();
      }

      // remove the failed payload
      auto failed_vtbid_it =
          std::find(index.vtbids.begin(), index.vtbids.end(), pid);
      VBK_ASSERT(failed_vtbid_it != index.vtbids.end() &&
                 "state corruption: the block does not contain the payload");
      index.vtbids.erase(failed_vtbid_it);

      return false;
    }
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

  auto pids = map_get_id(payloads);

  // check that we can add all payloads at once to guarantee atomicity
  auto& vtbids = index->getPayloadIds<VTB>();
  std::set<pid_t> existingPids(vtbids.begin(), vtbids.end());
  for (const auto& pid : pids) {
    if (!existingPids.insert(pid).second) {
      return state.Invalid(
          block_t::name() + "-duplicate-payloads",
          fmt::sprintf("Containing block=%s already contains payload %s.",
                       index->toPrettyString(),
                       pid.toHex()));
    }
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

  // temporal validation: the connecting BTC block must be added in 'index' or
  // earlier blocks
  bool areContextsValid = std::all_of(
      payloads.begin(), payloads.end(), [&](const payloads_t& payload) {
        return validateBTCContext(payload, state);
      });

  if (!areContextsValid) {
    // restore the tip
    if (!isOnActiveChain) {
      bool success = setState(*tip, state);
      VBK_ASSERT(success &&
                 "state corruption: failed to restore the best chain tip");
    }

    return state.Invalid(
        block_t::name() + "-btc-context-does-not-connect",
        fmt::sprintf("one of the payloads we attempted to add to block %s has "
                     "the BTC context that does not connect to the BTC tree",
                     index->toPrettyString()));
  }

  // apply payloads
  std::vector<pid_t> appliedPayloads;

  for (const auto& payload : payloads) {
    auto pid = payload.getId();

    bool added = addPayloadToAppliedBlock(*index, payload, state);
    if (!added) {
      // roll back previously applied payloads
      for (const auto& pidToRemove : reverse_iterate(appliedPayloads)) {
        storage_.removeVbkPayloadIndex(containingHash, pid.asVector());
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

void VbkBlockTree::payloadsToCommands(const payloads_t& p,
                                      std::vector<CommandPtr>& commands) {
  // process context blocks
  for (const auto& b : p.transaction.blockOfProofContext) {
    addBlock(btc(), b, p.containingBlock.height, commands);
  }
  // process block of proof
  addBlock(
      btc(), p.transaction.blockOfProof, p.containingBlock.height, commands);

  // add endorsement
  auto e = VbkEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddVbkEndorsement>(btc(), *this, std::move(e));
  commands.push_back(std::move(cmd));
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

  // recover `endorsedBy`
  auto window = std::max(
      0, index.getHeight() - param_->getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsedBy(*this, chain, *current, state)) {
    return state.Error("bad-endorsements");
  }

  storage_.addBlockToIndex(*current);

  return true;
}

void VbkBlockTree::removeSubtree(VbkBlockTree::index_t& toRemove) {
  storage_.removePayloadsIndex(toRemove);
  BaseBlockTree::removeSubtree(toRemove);
}

VbkBlockTree::VbkBlockTree(const VbkChainParams& vbkp,
                           const BtcChainParams& btcp,
                           PayloadsStorage& storagePayloads)
    : VbkTree(vbkp),
      cmp_(std::make_shared<BtcTree>(btcp), btcp, vbkp, storagePayloads),
      storage_(storagePayloads) {}

bool VbkBlockTree::loadTip(const Blob<24>& hash, ValidationState& state) {
  if (!base::loadTip(hash, state)) {
    return false;
  }

  auto* tip = activeChain_.tip();
  VBK_ASSERT(tip);
  while (tip) {
    tip->setFlag(BLOCK_APPLIED);
    tip = tip->pprev;
  }

  return true;
}

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands(
    const typename VbkBlockTree::index_t& index, VbkBlockTree& tree) {
  return loadCommandsStorage<VbkBlockTree, VTB>(DB_VTB_PREFIX, index, tree);
}

template <>
void removePayloadsFromIndex(BlockIndex<VbkBlock>& index,
                             const CommandGroup& cg) {
  VBK_ASSERT(cg.payload_type_name == VTB::name());
  auto& payloads = index.template getPayloadIds<VTB>();
  auto it = std::find(payloads.rbegin(), payloads.rend(), cg.id);
  VBK_ASSERT(it != payloads.rend());
  index.removePayloadId<VTB>(cg.id);
}

}  // namespace altintegration
