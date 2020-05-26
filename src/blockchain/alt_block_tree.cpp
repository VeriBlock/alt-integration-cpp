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

namespace altintegration {

AltTree::index_t* AltTree::insertBlockHeader(const AltBlock& block) {
  auto hash = block.getHash();
  index_t* current = base::getBlockIndex(hash);
  if (current != nullptr) {
    // it is a duplicate
    return current;
  }

  current = doInsertBlockHeader(std::make_shared<AltBlock>(block));
  current->setFlag(BLOCK_VALID_TREE);
  return current;
}

bool AltTree::bootstrap(ValidationState& state) {
  if (!base::blocks_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!base::blocks_.empty() && (getBlockIndex(block.getHash()) == nullptr)) {
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
               index.toPrettyString());
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
    assert(ret);
    (void)ret;
  }

  auto& c = index.commands;
  for (const auto& p : payloads) {
    c.emplace_back();
    auto& g = c.back();
    g.id = p.getId();
    payloadsToCommands(p, g.commands);
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
    removePayloads(*index, {p});
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

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
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
  VBK_LOG_DEBUG("Pop Difficulty=%s for block %s",
                popDifficulty.toPrettyString(),
                index->toPrettyString());
  VBK_LOG_DEBUG("Paying to %d addresses", ret.size());
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

void AltTree::determineBestChain(Chain<index_t>& currentBest,
                                 index_t& indexNew,
                                 ValidationState& state,
                                 bool isBootstrap) {
  if (currentBest.tip() == &indexNew) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!indexNew.isValid()) {
    return;
  }

  auto currentTip = currentBest.tip();
  if (currentTip == nullptr) {
    bool ret = setTip(indexNew, state, isBootstrap);
    assert(ret);
    (void)ret;
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
                             const std::vector<payloads_t>& payloads) {
  auto* index = base::getBlockIndex(hash);
  if (!index) {
    throw std::logic_error("removePayloads is called on unknown ALT block: " +
                           HexStr(hash));
  }

  return removePayloads(*index, payloads);
}

void AltTree::removePayloads(index_t& index,
                             const std::vector<payloads_t>& payloads) {
  VBK_LOG_INFO("%s remove %d payloads from %s",
               block_t::name(),
               payloads.size(),
               index.toPrettyString());
  if (!index.pprev) {
    // we do not add payloads to genesis block, therefore we do not have to
    // remove them
    return;
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    assert(index.pprev && "can not remove payloads from genesis block");
    ValidationState dummy;
    bool ret = setTip(*index.pprev, dummy, false);
    assert(ret);
    (void)ret;
  }

  auto& c = index.commands;

  // we need only ids, so save some cpu cycles by calculating ids once
  std::vector<uint256> pids = map_vector<payloads_t, uint256>(
      payloads, [](const payloads_t& p) { return p.getId(); });

  // iterate over payloads backwards
  for (const auto& pid : make_reversed(pids.begin(), pids.end())) {
    // find every payloads in command group (search backwards, as it is likely
    // to be faster)
    auto it = std::find_if(c.rbegin(), c.rend(), [&pid](const CommandGroup& g) {
      return g.id == pid;
    });

    if (it == c.rend()) {
      // not found
      continue;
    }

    // if this payloads invalidated subtree, we have to re-validate it again
    if (!it->valid) {
      revalidateSubtree(index, BLOCK_FAILED_POP, true);
    }

    // TODO(warchant): fix inefficient erase (does reallocation for every
    // payloads item)
    auto toRemove = --(it.base());
    c.erase(toRemove);
  }
}

void AltTree::payloadsToCommands(const typename AltTree::payloads_t& p,
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

    auto e = VbkEndorsement::fromContainerPtr(p);
    auto cmd = std::make_shared<AddVbkEndorsement>(vbk(), *this, std::move(e));
    commands.push_back(std::move(cmd));
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
                 to.toPrettyString(),
                 (vbk().getBestChain().tip()
                      ? vbk().getBestChain().tip()->toPrettyString()
                      : "<empty>"),
                 (btc().getBestChain().tip()
                      ? btc().getBestChain().tip()->toPrettyString()
                      : "<empty>"));
    activeChain_.setTip(&to);
    tryAddTip(&to);
  } else {
    assert(!to.isValid());
  }

  // true if tip has been changed
  return changeTip;
}

}  // namespace altintegration
