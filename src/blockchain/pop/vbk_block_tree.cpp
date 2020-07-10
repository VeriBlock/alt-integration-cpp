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
#include <veriblock/storage/blockchain_storage_util.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(index_t& candidate,
                                      ValidationState& state) {
  auto bestTip = getBestChain().tip();
  VBK_ASSERT(bestTip != nullptr && "must be bootstrapped");

  if (bestTip->height > candidate.height + param_->getMaxReorgBlocks()) {
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

  for (const auto& pid : pids) {
    auto it = std::find(index.vtbids.begin(), index.vtbids.end(), pid);
    // using an assert because throwing breaks atomicity
    // if there are multiple pids
    VBK_ASSERT(it != index.vtbids.end() &&
               "could not find the payload to remove");

    if (!storagePayloads_.isValid<VTB, index_t>(pid, index)) {
      revalidateSubtree(index, BLOCK_FAILED_POP, false);
    }

    index.vtbids.erase(it);
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

  auto vtbid_it = std::find(index.vtbids.begin(), index.vtbids.end(), pid);
  VBK_ASSERT(vtbid_it != index.vtbids.end() &&
             "state corruption: the block does not contain the payload");

  // removing an invalid payload might render the block valid
  if (!storagePayloads_.isValid<VTB, index_t>(pid, index)) {
    revalidateSubtree(index, BLOCK_FAILED_POP, false);
  }

  bool isApplied = activeChain_.contains(&index);

  if (isApplied) {
    auto cmdGroups = storagePayloads_.loadCommands<VbkBlockTree>(index, *this);

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

  index.vtbids.erase(vtbid_it);

  if (shouldDetermineBestChain) {
    updateTips();
  }
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

  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return state.Invalid(
        block_t::name() + "-bad-chain",
        fmt::sprintf("Containing block=%s is added on top of invalid chain",
                     index->toPrettyString()));
  }

  // check that we can add all payloads at once to guarantee atomicity
  std::set<pid_t> existingPids(index->vtbids.begin(), index->vtbids.end());
  for (const auto& payload : payloads) {
    auto pid = payload.getId();
    if (!existingPids.insert(pid).second) {
      return state.Invalid(
          block_t::name() + "-duplicate-payloads",
          fmt::sprintf("Containing block=%s already contains payload %s.",
                       index->toPrettyString(),
                       pid.toHex()));
    }
  }

  auto tip = activeChain_.tip();

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    VBK_ASSERT(tip != nullptr);
    auto window = (std::max)(0, tip->height - index->height);
    if (window >= param_->getHistoryOverwriteLimit()) {
      return state.Invalid(
          block_t::name() + "-too-late",
          fmt::sprintf(
              "Containing block=%s is too much behind "
              "of active chain tip. Diff %d is more than allowed %d blocks.",
              index->toShortPrettyString(),
              window,
              param_->getHistoryOverwriteLimit()));
    }

    ValidationState dummy;
    bool success = setState(*index->pprev, dummy);
    VBK_ASSERT(success &&
               "state corruption: failed to roll back the best chain tip");
  }

  for (const auto& payload : payloads) {
    auto pid = payload.getId();

    index->vtbids.push_back(pid);
    storagePayloads_.savePayloads(payload);
  }

  // don't defer fork resolution in the acceptBlock+addPayloads flow until the
  // validation hole is plugged
  doUpdateAffectedTips(*index, state);

  if (index->isValid()) {
    return true;
  }

  // roll back our attempted changes
  for (const auto& payload : payloads) {
    unsafelyRemovePayload(
        *index, payload.getId(), /*shouldDetermineBestChain =*/false);
  }

  // restore the tip
  if (isOnActiveChain) {
    ValidationState dummy;
    bool success = setState(*tip, dummy);
    VBK_ASSERT(success &&
               "state corruption: failed to restore the best chain tip");
  }

  return false;
}

void VbkBlockTree::payloadsToCommands(const payloads_t& p,
                                      std::vector<CommandPtr>& commands) {
  // process context blocks
  for (const auto& b : p.transaction.blockOfProofContext) {
    addBlock(btc(), b, commands);
  }
  // process block of proof
  addBlock(btc(), p.transaction.blockOfProof, commands);

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
  auto window =
      std::max(0, index.height - param_->getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsedBy(*this, chain, *current, state)) {
    // we already inserted block, so remove it
    removeLeaf(*current, false);
    return state.Invalid("load-block");
  }

  return true;
}

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands(
    const typename VbkBlockTree::index_t& index, VbkBlockTree& tree) {
  return loadCommandsStorage<VbkBlockTree, VTB>(index, tree);
}

bool removeId(std::vector<uint256>& pop, const uint256& id) {
  auto it = std::find(pop.rbegin(), pop.rend(), id);
  if (it == pop.rend()) {
    return false;
  }

  auto toRemove = --(it.base());
  pop.erase(toRemove);
  return true;
}

template <>
void removePayloadsFromIndex(BlockIndex<VbkBlock>& index,
                             const CommandGroup& cg) {
  VBK_ASSERT(cg.payload_type_name == VTB::name());
  bool success = removeId(index.vtbids, cg.id);
  VBK_ASSERT(success);
}

}  // namespace altintegration
