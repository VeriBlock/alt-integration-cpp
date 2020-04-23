// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cassert>
#include <utility>
#include <vector>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

static bool isKeystoneRound(const PopRewardsParams& rewardParams,
                            uint32_t payoutRound) {
  return payoutRound == rewardParams.keystoneRound();
}

// test if block is placed inside payout round following keystone
static bool isFirstRoundAfterKeystone(const AltChainParams& altParams,
                                      uint32_t height) {
  uint32_t blockAfterKeystone = height % altParams.getKeystoneInterval();
  if (altParams.getRewardParams().payoutRounds() == 0) return true;
  return blockAfterKeystone / altParams.getRewardParams().payoutRounds() == 0;
}

static PopRewardsBigDecimal getRoundRatio(const PopRewardsParams& rewardParams,
                                          uint32_t payoutRound) {
  return rewardParams.roundRatios().at(payoutRound);
}

static PopRewardsBigDecimal getMaxScoreThreshold(
    const PopRewardsParams& rewardParams, uint32_t payoutRound) {
  if (isKeystoneRound(rewardParams, payoutRound)) {
    return rewardParams.maxScoreThresholdKeystone();
  }
  return rewardParams.maxScoreThresholdNormal();
}

// slope is how the payout is decreased for each additional block score
static PopRewardsBigDecimal getRoundSlope(const PopRewardsParams& rewardParams,
                                          uint32_t payoutRound) {
  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();
  PopRewardsBigDecimal slopeRatio = curveParams.slopeNormal();
  if (payoutRound == rewardParams.keystoneRound()) {
    slopeRatio = curveParams.slopeKeystone();
  }
  return slopeRatio;
}

// apply the reward curve to the score and subtract it from the current round
// multiplier
static PopRewardsBigDecimal calculateSlopeRatio(
    const PopRewardsParams& rewardParams,
    PopRewardsBigDecimal score,
    uint32_t payoutRound) {
  PopRewardsBigDecimal slope = getRoundSlope(rewardParams, payoutRound);
  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();

  assert(score >= curveParams.startOfSlope());

  PopRewardsBigDecimal scoreDecrease =
      slope * (score - curveParams.startOfSlope());
  PopRewardsBigDecimal maxScoreDecrease = 1.0;
  if (scoreDecrease > maxScoreDecrease) {
    scoreDecrease = maxScoreDecrease;
  }
  return (maxScoreDecrease - scoreDecrease);
}

// rounds for blocks are [3, 1, 2, 0, 1, 2, 0, 1, 2, 0, 3, ...]
uint32_t PopRewardsCalculator::getRoundForBlockNumber(uint32_t height) const {
  if (height % altParams_->getKeystoneInterval() == 0) {
    return altParams_->getRewardParams().keystoneRound();
  }

  if (altParams_->getRewardParams().payoutRounds() <= 1) {
    return 0;
  }

  assert(height > 0);
  uint32_t round = (height % altParams_->getKeystoneInterval()) %
                   (altParams_->getRewardParams().payoutRounds() - 1);
  return round;
}

PopRewardsBigDecimal PopRewardsCalculator::getScoreMultiplierFromRelativeBlock(
    int relativeBlock) const {
  if (relativeBlock < 0 || relativeBlock >= (int)altParams_->getRewardParams()
                                                .relativeScoreLookupTable()
                                                .size()) {
    return 0.0;
  }

  return altParams_->getRewardParams()
      .relativeScoreLookupTable()[relativeBlock];
}

PopRewardsBigDecimal PopRewardsCalculator::calculateMinerRewardWithWeight(
    uint32_t height,
    PopRewardsBigDecimal endorsementWeight,
    PopRewardsBigDecimal scoreForThisBlock,
    PopRewardsBigDecimal difficulty) const {
  if (scoreForThisBlock == 0.0) {
    return 0.0;
  }

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }

  uint32_t payoutRound = getRoundForBlockNumber(height);
  PopRewardsBigDecimal scoreToDifficulty = scoreForThisBlock / difficulty;
  PopRewardsCurveParams curveParams =
      altParams_->getRewardParams().getCurveParams();
  PopRewardsBigDecimal roundRatio =
      getRoundRatio(altParams_->getRewardParams(), payoutRound);

  // penalty multiplier
  PopRewardsBigDecimal slope = 1.0;

  if (scoreToDifficulty > curveParams.startOfSlope()) {
    PopRewardsBigDecimal maxScoreThreshold =
        getMaxScoreThreshold(altParams_->getRewardParams(), payoutRound);
    if (scoreToDifficulty > maxScoreThreshold) {
      scoreToDifficulty = maxScoreThreshold;
    }

    // Note that this reward per point is not the true rewardWithSlope if the
    // score to difficulty ratio is greater than the max reward threshold. Past
    // the max reward threshold, the block reward ceases to grow, but is split
    // amongst a larger number of participants.
    slope = calculateSlopeRatio(
        altParams_->getRewardParams(), scoreToDifficulty, payoutRound);
  }

  return slope * endorsementWeight * roundRatio / difficulty;
}

// we calculate the reward for a given miner
PopRewardsBigDecimal PopRewardsCalculator::calculateMinerReward(
    uint32_t height,
    uint32_t vbkRelativeHeight,
    PopRewardsBigDecimal scoreForThisBlock,
    PopRewardsBigDecimal difficulty) const {
  if (scoreForThisBlock == 0.0) {
    return 0.0;
  }

  // Special case for the first ROUND 3 after keystone - do not adjust for score
  // to difficulty ratio
  uint32_t roundNumber = getRoundForBlockNumber(height);
  auto endorsementLevelWeight =
      getScoreMultiplierFromRelativeBlock(vbkRelativeHeight);

  if (altParams_->getRewardParams().flatScoreRoundUse() &&
      roundNumber == altParams_->getRewardParams().flatScoreRound() &&
      isFirstRoundAfterKeystone(*altParams_, height)) {
    return calculateMinerRewardWithWeight(
        height, endorsementLevelWeight, 1.0, 1.0);
  }

  return calculateMinerRewardWithWeight(
      height, endorsementLevelWeight, std::move(scoreForThisBlock), std::move(difficulty));
}

// getter for altchain parameters
const AltChainParams& PopRewardsCalculator::getAltParams() const noexcept {
  return *altParams_;
}

}  // namespace altintegration
