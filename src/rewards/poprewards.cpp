// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cassert>
#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

static int getBestPublicationHeight(const BlockIndex<AltBlock>& endorsedBlock,
                                    const VbkBlockTree& vbk_tree) {
  int bestPublication = -1;
  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbk_tree.getBlockIndex(e->blockOfProof);
    if (!vbk_tree.getBestChain().contains(b)) continue;
    if (b->height < bestPublication || bestPublication < 0)
      bestPublication = b->height;
  }
  return bestPublication;
}

PopRewardsBigDecimal PopRewards::scoreFromEndorsements(
    const VbkBlockTree& vbk_tree,
    const BlockIndex<AltBlock>& endorsedBlock) const {
  PopRewardsBigDecimal totalScore = 0.0;
  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(endorsedBlock, vbk_tree);
  if (bestPublication < 0) return totalScore;

  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbk_tree.getBlockIndex(e->blockOfProof);
    if (!vbk_tree.getBestChain().contains(b)) continue;
    int relativeHeight = b->height - bestPublication;
    assert(relativeHeight >= 0);
    totalScore +=
        calculator_.getScoreMultiplierFromRelativeBlock(relativeHeight);
  }
  return totalScore;
}

PopRewardsBigDecimal PopRewards::calculateDifficulty(
    const VbkBlockTree& vbk_tree, const BlockIndex<AltBlock>& tip) const {
  PopRewardsBigDecimal difficulty = 0.0;
  auto rewardParams = calculator_.getAltParams().getRewardParams();
  const BlockIndex<AltBlock>* currentBlock = tip.pprev;

  for (size_t i = 0; i < rewardParams.difficultyAveragingInterval(); i++) {
    if (currentBlock == nullptr) break;
    difficulty += scoreFromEndorsements(vbk_tree, *currentBlock);
    currentBlock = currentBlock->pprev;
  }

  difficulty /=
      static_cast<uint64_t>(rewardParams.difficultyAveragingInterval());

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

std::map<std::vector<uint8_t>, int64_t> PopRewards::calculatePayouts(
    const VbkBlockTree& vbk_tree,
    const BlockIndex<AltBlock>& endorsedBlock,
    const PopRewardsBigDecimal& popDifficulty) {
  std::map<std::vector<uint8_t>, int64_t> rewards{};
  int bestPublication = getBestPublicationHeight(endorsedBlock, vbk_tree);
  if (bestPublication < 0) {
    return rewards;
  }

  auto blockScore = scoreFromEndorsements(vbk_tree, endorsedBlock);

  // pay reward for each of the endorsements
  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbk_tree.getBlockIndex(e->blockOfProof);
    if (!vbk_tree.getBestChain().contains(b)) continue;

    int veriBlockHeight = b->height;
    int relativeHeight = veriBlockHeight - bestPublication;
    assert(relativeHeight >= 0);
    auto minerReward = calculator_.calculateMinerReward(
        endorsedBlock.height, relativeHeight, blockScore, popDifficulty);
    rewards[e->payoutInfo] += minerReward.value.getLow64();
  }
  return rewards;
}

}  // namespace altintegration
