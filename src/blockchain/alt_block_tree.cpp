// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <unordered_set>
#include <veriblock/blockchain/commands/commands.hpp>

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
    return state.Invalid("bad-block", "Can't find containing block");
  }

  if (!index->pprev) {
    return state.Invalid("bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index->isValid()) {
    return state.Invalid("bad-chain",
                         "Containing block has been marked as invalid");
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    assert(ret);
    (void)ret;
  }

  for (const auto& p : payloads) {
    index->commands.emplace_back();
    auto& g = index->commands.back();
    g.id = p.getId();
    payloadsToCommands(*this, p, g.commands);
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
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!index->isValid()) {
    return state.Invalid("bad-chain", "One of previous blocks is invalid");
  }

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
      alt_config_->getRewardParams().rewardSettlementInterval());
  if (endorsedBlock == nullptr) {
    state.Error("Not enough blocks to get the endorsed block");
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(vbk(), *endorsedBlock);
  return rewards_.calculatePayouts(vbk(), *endorsedBlock, popDifficulty);
}

std::string AltTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  std::string pad(level, ' ');
  ss << pad << "AltTree{blocks=" << base::blocks_.size() << "\n";
  ss << base::toPrettyString(level + 2) << "\n";
  ss << cmp_.toPrettyString(level + 2) << "\n";
  return ss.str();
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

  // edge case: connected block is one of 'next' blocks after our current best
  if (indexNew.getAncestor(currentTip->height) == currentTip) {
    // an attempt to connect a NEXT block
    this->setTip(indexNew, state, false);
    return;
  }

  int result = cmp_.comparePopScore(*this, indexNew, state);
  if (result < 0) {
    // activeChain in AltTree reflects currently applied POP state.
    // setState has already been executed
    bool ret = this->setTip(indexNew, state, true);
    assert(ret);
    (void)ret;
  }

  // for other cases, we don't update pop score of active chain, so it remains
  // as is, even though there are regular alt blocks on top of current tip.
}

int AltTree::comparePopScore(const AltBlock::hash_t& hcurrent,
                             const AltBlock::hash_t& hother) {
  auto* index = getBlockIndex(hcurrent);
  if (!index) {
    throw std::logic_error("AltTree: unknown 'current' block");
  }

  auto* other = getBlockIndex(hother);
  if (!other) {
    throw std::logic_error("AltTree: unknown 'other' block");
  }

  ValidationState state;
  // set current state to match 'hcurrent'
  bool ret = setTip(*index, state, false);
  if (!ret) {
    throw std::logic_error(
        "AltTree: setState(hcurrent) failed. This means that current active "
        "chain failed. Most likely caller made a mistake.");
  }

  // compare current active chain to other chain
  int result = cmp_.comparePopScore(*this, *other, state);
  if (result < 0) {
    // other chain is better, change current state to 'other'
    ret = setTip(*other, state, /* skipSetState=*/true);
    assert(ret);
  }

  (void)ret;

  // our pop best chain has not changed, so do nothing here
  return result;
}

template <>
void payloadsToCommands<AltTree>(AltTree& tree,
                                 const typename AltTree::payloads_t& p,
                                 std::vector<CommandPtr>& commands) {
  // first, add vbk context
  for (const auto& b : p.popData.vbk_context) {
    addBlock(tree.vbk(), b, commands);
  }

  // second, add all VTBs
  for (const auto& vtb : p.popData.vtbs) {
    auto cmd = std::make_shared<AddVTB>(tree, vtb);
    commands.push_back(std::move(cmd));
  }

  // third, add ATV endorsement
  if (p.popData.hasAtv) {
    addBlock(tree.vbk(), p.popData.atv.containingBlock, commands);

    auto e = VbkEndorsement::fromContainerPtr(p);
    auto cmd =
        std::make_shared<AddVbkEndorsement>(tree.vbk(), tree, std::move(e));
    commands.push_back(std::move(cmd));
  }
}

}  // namespace altintegration
