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
    if (b->getHeight() < bestPublication || bestPublication < 0)
      bestPublication = b->getHeight();
  }
  return bestPublication;
}

PopRewardsBigDecimal PopRewards::scoreFromEndorsements(
    const BlockIndex<AltBlock>& endorsedBlock) {
  PopRewardsBigDecimal totalScore = 0.0;
  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(endorsedBlock, *vbkTree_);
  if (bestPublication < 0) return totalScore;

  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbkTree_->getBlockIndex(e->blockOfProof);
    if (!vbkTree_->getBestChain().contains(b)) continue;
    int relativeHeight = b->getHeight() - bestPublication;
    assert(relativeHeight >= 0);
    totalScore +=
        calculator_.getScoreMultiplierFromRelativeBlock(relativeHeight);
  }
  return totalScore;
}

PopRewardsBigDecimal PopRewards::calculateDifficulty(
    const BlockIndex<AltBlock>& tip) {
  PopRewardsBigDecimal difficulty = 0.0;
  auto& params = calculator_.getAltParams().getPayoutParams();
  const BlockIndex<AltBlock>* currentBlock = tip.pprev;

  for (size_t i = 0; i < params.difficultyAveragingInterval(); i++) {
    if (currentBlock == nullptr) break;
    difficulty += scoreFromEndorsements(*currentBlock);
    currentBlock = currentBlock->pprev;
  }

  difficulty /= static_cast<uint64_t>(params.difficultyAveragingInterval());

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

PopPayouts PopRewards::calculatePayoutsInner(
    const BlockIndex<AltBlock>& endorsedBlock,
    const PopRewardsBigDecimal& endorsedBlockScore,
    const PopRewardsBigDecimal& popDifficulty) {
  PopPayouts payouts{};
  int bestPublication = getBestPublicationHeight(endorsedBlock, *vbkTree_);
  if (bestPublication < 0) {
    return payouts;
  }

  // precalculate block reward - it helps calculating each miner's reward
  auto blockReward = calculator_.calculateBlockReward(
      endorsedBlock.getHeight(), endorsedBlockScore, popDifficulty);

  // pay reward for each of the endorsements
  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbkTree_->getBlockIndex(e->blockOfProof);
    if (!vbkTree_->getBestChain().contains(b)) continue;

    int veriBlockHeight = b->getHeight();
    int relativeHeight = veriBlockHeight - bestPublication;
    assert(relativeHeight >= 0);
    auto minerReward = calculator_.calculateMinerReward(
        relativeHeight, endorsedBlockScore, blockReward);

    payouts.add(PopPayoutValue{e->payoutInfo, minerReward.value.getLow64()});
  }
  return payouts;
}

PopPayouts PopRewards::calculatePayouts(
    const BlockIndex<AltBlock>& endorsedBlock) {
  auto blockScore = scoreFromEndorsements(endorsedBlock);
  auto popDifficulty = calculateDifficulty(endorsedBlock);
  return calculatePayoutsInner(endorsedBlock, blockScore, popDifficulty);
}

}  // namespace altintegration
