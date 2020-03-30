#include <cassert>
#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

static int getBestPublicationHeight(const BlockIndex<AltBlock>& block,
                                    const VbkBlockTree& vbk_tree) {
  int bestPublication = -1;
  for (const auto& e : block.containingEndorsements) {
    auto* b = vbk_tree.getBlockIndex(e.second->blockOfProof);
    if (!vbk_tree.getBestChain().contains(b)) continue;
    if (b->height < bestPublication || bestPublication < 0)
      bestPublication = b->height;
  }
  return bestPublication;
}

PopRewardsBigDecimal PopRewards::scoreFromEndorsements(
    const BlockIndex<AltBlock>& block) const {
  PopRewardsBigDecimal totalScore = 0.0;

  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(block, vbk_tree_);
  if (bestPublication < 0) return totalScore;

  for (const auto& e : block.containingEndorsements) {
    auto* b = vbk_tree_.getBlockIndex(e.second->blockOfProof);
    if (!vbk_tree_.getBestChain().contains(b)) continue;
    int relativeHeight = b->height - bestPublication;
    assert(relativeHeight >= 0);
    totalScore +=
        calculator_.getScoreMultiplierFromRelativeBlock(relativeHeight);
  }
  return totalScore;
}

PopRewardsBigDecimal PopRewards::calculateDifficulty(
    const BlockIndex<AltBlock>& tip) const {
  PopRewardsBigDecimal difficulty = 0.0;

  const BlockIndex<AltBlock>* currentBlock = &tip;
  // rewind rewardSettlementInterval blocks back in the past
  for (size_t i = 0; i < rewardParams_.rewardSettlementInterval(); i++) {
    currentBlock = currentBlock->pprev;
    if (currentBlock == nullptr) {
      throw std::logic_error(
          "amount of blocks must be higher or equal than "
          "rewardSettlementInterval");
    }
  }

  for (size_t i = 0; i < rewardParams_.difficultyAveragingInterval(); i++) {
    difficulty += scoreFromEndorsements(*currentBlock);
    currentBlock = currentBlock->pprev;
    if (currentBlock == nullptr) break;
  }

  difficulty /= (uint64_t)rewardParams_.difficultyAveragingInterval();

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

std::vector<PopRewardPayout> PopRewards::calculatePayouts(
    const BlockIndex<AltBlock>& block,
    PopRewardsBigDecimal popDifficulty) {

  std::vector<PopRewardPayout> rewards{};
  int bestPublication = getBestPublicationHeight(block, vbk_tree_);
  if (bestPublication < 0) return rewards;

  auto blockScore = scoreFromEndorsements(block);

  // we have the total reward per block in blockReward. Let's distribute it
  for (const auto& e : block.containingEndorsements) {
    auto* b = vbk_tree_.getBlockIndex(e.second->blockOfProof);
    if (!vbk_tree_.getBestChain().contains(b)) continue;

    int veriBlockHeight = b->height;
    int relativeHeight = veriBlockHeight - bestPublication;
    assert(relativeHeight >= 0);
    auto minerReward = calculator_.calculateRewardForMiner(
        block.height, relativeHeight, blockScore, popDifficulty);

    PopRewardPayout reward{};
    reward.reward = minerReward.getIntegerFraction();
    reward.miner = e.second->payoutInfo;
    rewards.push_back(reward);
  }
  return rewards;
}

}  // namespace altintegration
