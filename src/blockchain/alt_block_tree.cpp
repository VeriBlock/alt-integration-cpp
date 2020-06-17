// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <unordered_set>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/reversed_range.hpp>

#include "veriblock/algorithm.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/stateless_validation.hpp"
#include <veriblock/blockchain/blockchain_storage_util.hpp>

namespace altintegration {

bool AltTree::bootstrap(ValidationState& state) {
  if (!base::blocks_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(std::make_shared<AltBlock>(std::move(block)));

  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  if (!base::blocks_.empty() && (getBlockIndex(index->getHash()) == nullptr)) {
    return state.Error("block-index-no-genesis");
  }

  determineBestChain(base::activeChain_, *index, state, true);

  tryAddTip(index);

  return true;
}

bool AltTree::addPayloads(const AltBlock::hash_t& containing,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  auto* index = getBlockIndex(containing);
  if (!index) {
    return state.Invalid(block_t::name() + "-bad-block",
                         "Can't find containing block");
  }
  return addPayloads(*index, payloads, state);
}

bool AltTree::addPayloads(index_t& index,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  VBK_LOG_INFO("%s add %d payloads to block %s",
               block_t::name(),
               payloads.size(),
               index.toShortPrettyString());
  if (!index.pprev) {
    return state.Invalid(block_t::name() + "-bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index.isValid()) {
    return state.Invalid(block_t::name() + "-bad-chain",
                         "Containing block has been marked as invalid");
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool ret = setTip(*index.pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  std::set<pid_t> existingPids(index.payloadIds.begin(),
                               index.payloadIds.end());
  for (const auto& p : payloads) {
    auto pid = p.getId();
    if (!existingPids.insert(pid).second) {
      return state.Invalid(
          block_t::name() + "-duplicate-payloads",
          fmt::sprintf("Containing block=%s already contains payload %s.",
                       index.toPrettyString(),
                       pid.toHex()));
    }
    index.payloadIds.push_back(pid);
    storagePayloads_.savePayloads(p);
  }

  return true;
}

bool AltTree::validatePayloads(const AltBlock& block,
                               const payloads_t& p,
                               ValidationState& state) {
  return validatePayloads(block.getHash(), p, state);
}

bool AltTree::validatePayloads(const AltBlock::hash_t& block_hash,
                               const payloads_t& p,
                               ValidationState& state) {
  auto* index = getBlockIndex(block_hash);

  if (!index) {
    return state.Invalid(block_t::name() + "bad-block",
                         "Can't find containing block");
  }

  if (!addPayloads(*index, {p}, state)) {
    VBK_LOG_DEBUG(
        "%s Can not add payloads: %s", block_t::name(), state.toString());
    return state.Invalid(block_t::name() + "-addPayloadsTemporarily");
  }

  if (!setState(*index, state)) {
    VBK_LOG_DEBUG("%s Statefully invalid payloads: %s",
                  block_t::name(),
                  state.toString());
    removePayloads(*index, {p.getId()});
    return state.Invalid(block_t::name() + "-addPayloadsTemporarily");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
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
    return state.Invalid(block_t::name() + "-bad-chain",
                         "One of previous blocks is invalid. Status=(" +
                             std::to_string(index->status) + ")");
  }

  tryAddTip(index);

  return true;
}

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& tip, ValidationState& state) {
  auto* index = getBlockIndex(tip);
  if (index == nullptr) {
    state.Error("Block not found");
    return {};
  }

  auto* endorsedBlock = index->getAncestorBlocksBehind(
      alt_config_->getEndorsementSettlementInterval());
  if (endorsedBlock == nullptr) {
    state.Error("Not enough blocks to get the endorsed block");
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

void AltTree::payloadsToCommands(const payloads_t& p,
                                 std::vector<CommandPtr>& commands) {
  // first, add vbk context
  for (const auto& b : p.popData.vbk_context) {
    addBlock(vbk(), b, commands);
  }

  // second, add all VTBs
  for (const auto& vtb : p.popData.vtbs) {
    auto cmd = std::make_shared<AddVTB>(*this, vtb);
    commands.push_back(std::move(cmd));
  }

  // third, add ATV endorsement
  if (p.popData.hasAtv) {
    addBlock(vbk(), p.popData.atv.containingBlock, commands);

    auto e = AltEndorsement::fromContainerPtr(p);
    auto cmd = std::make_shared<AddAltEndorsement>(vbk(), *this, std::move(e));
    commands.push_back(std::move(cmd));
  }
}

bool AltTree::saveToStorage(PopStorage& storage, ValidationState& state) {
  saveBlocks(storage, vbk().btc());
  saveBlocks(storage, vbk());
  saveBlocks(storage, *this);
  return state.IsValid();
}

bool AltTree::loadFromStorage(const PopStorage& storage,
                              ValidationState& state) {
  bool ret = loadBlocks(storage, vbk().btc(), state);
  if (!ret) return state.IsValid();
  ret = loadBlocks(storage, vbk(), state);
  if (!ret) return state.IsValid();
  return loadBlocks(storage, *this, state);
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

void AltTree::determineBestChain(Chain<index_t>& currentBest,
                                 index_t& indexNew,
                                 ValidationState& state,
                                 bool isBootstrap) {
  if (currentBest.tip() == &indexNew) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!indexNew.isValid()) {
    VBK_LOG_DEBUG("Candidate %s is invalid, skipping FR",
                  indexNew.toPrettyString());
    return;
  }

  auto currentTip = currentBest.tip();
  if (currentTip == nullptr) {
    VBK_LOG_DEBUG("Current tip is nullptr, candidate %s becomes new tip",
                  indexNew.toShortPrettyString());
    bool ret = setTip(indexNew, state, isBootstrap);
    VBK_ASSERT(ret);
    return;
  }

  // if tip==nullptr, then update tip.
  // else - do nothing. AltTree does not (yet) do fork resolution
}

int AltTree::comparePopScore(const AltBlock::hash_t& hleft,
                             const AltBlock::hash_t& hright) {
  auto* left = getBlockIndex(hleft);
  if (!left) {
    throw std::logic_error("AltTree: unknown 'left' block");
  }

  auto* right = getBlockIndex(hright);
  if (!right) {
    throw std::logic_error("AltTree: unknown 'other' block");
  }

  if (activeChain_.tip() != left) {
    throw std::logic_error(
        "AltTree: left fork must be applied. Call SetState(left) before fork "
        "resolution.");
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

void AltTree::removePayloads(const AltBlock::hash_t& hash,
                             const std::vector<pid_t>& pids) {
  auto* index = base::getBlockIndex(hash);
  if (!index) {
    throw std::logic_error("removePayloads is called on unknown ALT block: " +
                           HexStr(hash));
  }

  return removePayloads(*index, pids);
}

void AltTree::removePayloads(index_t& index, const std::vector<pid_t>& pids) {
  VBK_LOG_INFO("%s remove %d payloads from %s",
               block_t::name(),
               pids.size(),
               index.toShortPrettyString());
  if (!index.pprev) {
    // we do not add payloads to genesis block, therefore we do not have to
    // remove them
    return;
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    VBK_ASSERT(index.pprev && "can not remove payloads from genesis block");
    ValidationState dummy;
    bool ret = setTip(*index.pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  for (const auto& pid : pids) {
    auto it =
        std::find(index.payloadIds.begin(), index.payloadIds.end(), pid);
    if (it == index.payloadIds.end()) {
      // TODO: error message
      continue;
    }

    auto payloads = storagePayloads_.loadPayloads<payloads_t>(pid);
    if (!payloads.valid) {
      revalidateSubtree(index, BLOCK_FAILED_POP, false);
    }

    index.payloadIds.erase(it);
    //TODO: do we want to erase payloads from repository?
  }
}

bool AltTree::setTip(AltTree::index_t& to,
                     ValidationState& state,
                     bool skipSetState) {
  bool changeTip = true;
  if (!skipSetState) {
    changeTip = cmp_.setState(*this, to, state);
  }

  // edge case: if changeTip is false, then new block arrived on top of
  // current active chain, and this block has invalid commands
  if (changeTip) {
    VBK_LOG_INFO("ALT=\"%s\", VBK=\"%s\", BTC=\"%s\"",
                 to.toShortPrettyString(),
                 (vbk().getBestChain().tip()
                      ? vbk().getBestChain().tip()->toShortPrettyString()
                      : "<empty>"),
                 (btc().getBestChain().tip()
                      ? btc().getBestChain().tip()->toShortPrettyString()
                      : "<empty>"));
    activeChain_.setTip(&to);
    tryAddTip(&to);
  } else {
    VBK_ASSERT(!to.isValid());
  }

  // true if tip has been changed
  return changeTip;
}

template <>
ArithUint256 getBlockProof(const AltBlock&) {
  return 0;
}

}  // namespace altintegration
