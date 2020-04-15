#include <cassert>
#include <vector>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

// rounds for blocks are [3, 1, 2, 0, 1, 2, 0, 1, 2, 0, 3, ...]
static uint32_t getRoundForBlockNumber(const AltChainParams& altParams,
                                       uint32_t height) {
  if (height % altParams.getKeystoneInterval() == 0) {
    return altParams.getRewardParams().keystoneRound();
  }

  if (altParams.getRewardParams().payoutRounds() <= 1) {
    return 0;
  }

  assert(height > 0);
  uint32_t round = (height % altParams.getKeystoneInterval()) %
                   (altParams.getRewardParams().payoutRounds() - 1);
  return round;
}

static bool isKeystoneRound(const PopRewardsParams& rewardParams,
                            uint32_t payoutRound) {
  return payoutRound == rewardParams.keystoneRound();
}

// test if block is placed inside payout round following keystone
static bool isFirstRoundAfterKeystone(const AltChainParams& altParams,
                                      uint32_t height) {
  uint32_t blockAfterKeystone = height % altParams.getKeystoneInterval();
  if (altParams.getRewardParams().payoutRounds() == 0) return true;
  if (blockAfterKeystone / altParams.getRewardParams().payoutRounds() == 0)
    return true;
  return false;
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

static PopRewardsBigDecimal calculateMinerReward(
    const AltChainParams& altParams,
    uint32_t height,
    PopRewardsBigDecimal endorsementWeight,
    PopRewardsBigDecimal difficulty,
    PopRewardsBigDecimal score) {
  if (score == 0.0) {
    return 0.0;
  }

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }

  uint32_t payoutRound =
      getRoundForBlockNumber(altParams, height);
  PopRewardsBigDecimal scoreToDifficulty = score / difficulty;
  PopRewardsCurveParams curveParams = altParams.getRewardParams().getCurveParams();
  PopRewardsBigDecimal roundRatio =
      getRoundRatio(altParams.getRewardParams(), payoutRound);

  // penalty multiplier
  PopRewardsBigDecimal slope = 1.0;

  if (scoreToDifficulty > curveParams.startOfSlope()) {
    PopRewardsBigDecimal maxScoreThreshold =
        getMaxScoreThreshold(altParams.getRewardParams(), payoutRound);
    if (scoreToDifficulty > maxScoreThreshold) {
      scoreToDifficulty = maxScoreThreshold;
    }

    // Note that this reward per point is not the true rewardWithSlope if the
    // score to difficulty ratio is greater than the max reward threshold. Past
    // the max reward threshold, the block reward ceases to grow, but is split
    // amongst a larger number of participants.
    slope = calculateSlopeRatio(
        altParams.getRewardParams(), scoreToDifficulty, payoutRound);
  }

  return slope * endorsementWeight * roundRatio / difficulty;
}

PopRewardsBigDecimal PopRewardsCalculator::getScoreMultiplierFromRelativeBlock(
    int relativeBlock) const {
  if (relativeBlock < 0 ||
      relativeBlock >= (int)altParams_.getRewardParams().relativeScoreLookupTable().size()) {
    return 0.0;
  }

  return altParams_.getRewardParams().relativeScoreLookupTable()[relativeBlock];
}

// we calculate the reward for a given miner
PopRewardsBigDecimal PopRewardsCalculator::calculateRewardForMiner(
    uint32_t height,
    uint32_t vbkRelativeHeight,
    PopRewardsBigDecimal scoreForThisBlock,
    PopRewardsBigDecimal difficulty) const {
  if (scoreForThisBlock == 0.0) {
    return 0.0;
  }

  // Special case for the first ROUND 3 after keystone - do not adjust for score
  // to difficulty ratio
  uint32_t roundNumber = getRoundForBlockNumber(altParams_, height);
  auto endorsementLevelWeight =
      getScoreMultiplierFromRelativeBlock(vbkRelativeHeight);

  if (altParams_.getRewardParams().flatScoreRoundUse() &&
      roundNumber == altParams_.getRewardParams().flatScoreRound() &&
      isFirstRoundAfterKeystone(altParams_, height)) {
    return calculateMinerReward(
        altParams_, height, endorsementLevelWeight, 1.0, 1.0);
  }

  return calculateMinerReward(altParams_,
                              height,
                              endorsementLevelWeight,
                              difficulty,
                              scoreForThisBlock);
}

// getter for altchain parameters
const AltChainParams& PopRewardsCalculator::getAltParams() const noexcept {
  return altParams_;
}

}  // namespace altintegration
