// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <unordered_set>
#include <veriblock/blockchain/commands/commands.hpp>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/stateless_validation.hpp"
#include <veriblock/third_party/fmt/printf.h>

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
  return addPayloads(*index, payloads, state);
}

bool AltTree::addPayloads(index_t& index,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  if (!index.pprev) {
    return state.Invalid("bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index.isValid()) {
    return state.Invalid("bad-chain",
                         "Containing block has been marked as invalid");
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool ret = setTip(*index.pprev, dummy, false);
    assert(ret);
    (void)ret;
  }

  for (const auto& p : payloads) {
    index.commands.emplace_back();
    auto& g = index.commands.back();
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
    return state.Invalid("bad-block", "Can't find containing block");
  }

  if (!addPayloads(*index, {p}, state)) {
    return state.Invalid("addPayloadsTemporarily");
  }

  if (!setState(*index, state)) {
    removePayloads(*index, {p});
    index->unsetFlag(BlockStatus::BLOCK_FAILED_POP);
    return state.Invalid("addPayloadsTemporarily");
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
      alt_config_->getRewardParams().rewardSettlementInterval());
  if (endorsedBlock == nullptr) {
    state.Error("Not enough blocks to get the endorsed block");
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(vbk(), *endorsedBlock);
  return rewards_.calculatePayouts(vbk(), *endorsedBlock, popDifficulty);
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

namespace {

//! cost is a number of blocks that we have to unapply and apply to change state
//! from 'chain' to 'index'
size_t calculateStateChangeCost(Chain<BlockIndex<AltBlock>>& chain,
                                BlockIndex<AltBlock>& index) {
  auto tip = chain.tip();

  // already at this state, so cost is 0
  if (!tip || *tip == index) {
    return 0;
  }

  // case: index is a subchain
  if (chain.contains(&index)) {
    return 1;
  }

  // index is one of 'next' blocks
  if (index.getAncestor(tip->height) == tip) {
    return 2;
  }

  // index is on a fork
  size_t unapplyCost = 0;
  size_t applyCost = 0;

  auto* forkBlock = chain.findFork(&index);
  if (!forkBlock) {
    // unreachable node
    return (std::numeric_limits<size_t>::max)();
  }

  unapplyCost = tip->height - forkBlock->height;
  applyCost = index.height - forkBlock->height;

  return unapplyCost + applyCost;
}

}  // namespace

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

  // if 1, then ACTIVE=left, OTHER=right
  // if -1, then ACTIVE=right, OTHER=left
  int coefficient = 1;
  auto leftCost = calculateStateChangeCost(activeChain_, *left);
  auto rightCost = calculateStateChangeCost(activeChain_, *right);
  if (leftCost > rightCost) {
    // make 'right' to be the 'current' fork, and 'left' as 'other fork'
    std::swap(left, right);
    coefficient *= -1;
  }

  ValidationState state;
  // set current state to match 'hcurrent'
  bool ret = setTip(*left, state, false);
  if (!ret) {
    ret = setTip(*right, state, false);
    if (!ret) {
      throw std::logic_error("AltTree: both chains are invalid");
    }
    // left is invalid, right is valid

    // swap direction again
    std::swap(left, right);
    coefficient *= -1;
  }

  // compare current active chain to other chain
  int result = cmp_.comparePopScore(*this, *right, state);
  if (result < 0) {
    // other chain is better, change current state to 'other'
    activeChain_.setTip(right);
  }

  (void)ret;

  // our pop best chain has not changed, so do nothing here
  return coefficient * result;
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

  // remove all matched command groups
  auto& c = index.commands;
  c.erase(
      std::remove_if(c.begin(),
                     c.end(),
                     [&](const CommandGroup& g) {
                       for (const auto& p : payloads) {
                         if (g == p.getId()) {
                           if (!g.valid) {
                             base::revalidateSubtree(
                                 index, BlockStatus::BLOCK_FAILED_POP, false);
                           }
                           return true;
                         }
                       }

                       return false;
                     }),
      c.end());
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

}  // namespace altintegration
