#include <cassert>
#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

static int getBestPublicationHeight(
    const std::vector<VbkEndorsement>& endorsements,
    const std::shared_ptr<VbkBlockTree>& vbk_tree) {
  int bestPublication = -1;
  for (const VbkEndorsement& e : endorsements) {
    auto* block = vbk_tree->getBlockIndex(e.blockOfProof);
    if (block == nullptr) continue;

    if (block->height < bestPublication || bestPublication < 0)
      bestPublication = block->height;
  }
  return bestPublication;
}

static PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
    int relativeBlock, const PopRewardsParams& rewardParams) {
  if (relativeBlock < 0 ||
      relativeBlock >= (int)rewardParams.relativeScoreLookupTable().size()) {
    return 0.0;
  }

  return rewardParams.relativeScoreLookupTable()[relativeBlock];
}

PopRewardsBigDecimal PopRewards::scoreFromEndorsements(
    const AltBlock& endorsedBlock) {
  PopRewardsBigDecimal totalScore = 0.0;

  /// TODO: in Java we were limiting the search in containingBlocks list.
  ///      Maybe we should do the same here as well.
  auto endorsements = erepo_->get(endorsedBlock.getHash());

  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(endorsements, vbk_tree_);
  if (bestPublication < 0) return totalScore;

  for (const VbkEndorsement& e : endorsements) {
    auto* block = vbk_tree_->getBlockIndex(e.blockOfProof);
    if (block == nullptr) continue;
    int relativeHeight = block->height - bestPublication;
    assert(relativeHeight >= 0);
    totalScore += getScoreMultiplierFromRelativeBlock(relativeHeight, rewardParams_);
  }
  return totalScore;
}

PopRewardsBigDecimal PopRewards::calculateDifficulty(
    const std::vector<AltBlock>& blocks) {
  if (blocks.size() > rewardParams_.rewardSettlementInterval() +
                      rewardParams_.difficultyAveragingInterval()) {
    throw std::logic_error(
        "amount of blocks must be less or equal than "
        "rewardSettlementInterval + difficultyAveragingInterval");
  }

  if (blocks.size() < rewardParams_.rewardSettlementInterval()) {
    throw std::logic_error(
        "amount of blocks must be higher or equal than "
        "rewardSettlementInterval");
  }

  PopRewardsBigDecimal difficulty = 0.0;

  std::vector<AltBlock> sortedBlocks(blocks);
  std::sort(
      sortedBlocks.begin(),
      sortedBlocks.end(),
      [](const AltBlock& a, const AltBlock& b) { return a.height < b.height; });

  size_t averagingBlocks =
      sortedBlocks.size() - rewardParams_.rewardSettlementInterval();

  for (size_t i = 0; i < averagingBlocks; i++) {
    difficulty += scoreFromEndorsements(sortedBlocks[i]);
  }

  difficulty /= (uint64_t)rewardParams_.difficultyAveragingInterval();

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

std::vector<PopRewardPayout> PopRewards::calculatePayouts(
    uint32_t height,
    const AltBlock& endorsedBlock,
    PopRewardsBigDecimal popDifficulty) {

  std::vector<PopRewardPayout> rewards{};
  auto endorsements = erepo_->get(endorsedBlock.getHash());
  auto payloads = prepo_->get(endorsedBlock.getHash());
  int bestPublication = getBestPublicationHeight(endorsements, vbk_tree_);
  if (bestPublication < 0) return rewards;

  auto scoreForThisBlock = scoreFromEndorsements(endorsedBlock);
  auto blockReward = calculator_.calculatePopRewardForBlock(
      height, scoreForThisBlock, popDifficulty);

  // we have the total reward per block in blockReward. Let's distribute it
  // now.
  for (const VbkEndorsement& e : endorsements) {
    auto* block = vbk_tree_->getBlockIndex(e.blockOfProof);
    if (block == nullptr) continue;

    int veriBlockHeight = block->height;
    int relativeHeight = veriBlockHeight - bestPublication;
    assert(relativeHeight >= 0);
    auto endorsementLevelWeight =
        getScoreMultiplierFromRelativeBlock(relativeHeight, rewardParams_);
    auto rewardPerEndorsement =
        blockReward * endorsementLevelWeight / scoreForThisBlock;

    PopRewardPayout reward{};
    reward.reward = rewardPerEndorsement.getIntegerFraction();
    reward.miner = std::string(e.payoutInfo.begin(), e.payoutInfo.end());
    rewards.push_back(reward);
  }
  return rewards;
}

}  // namespace altintegration
