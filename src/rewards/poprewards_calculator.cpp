#include <cassert>
#include <vector>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

// rounds for blocks are [4, 1, 2, 3, 1, 2, 3, 1, 2, 3, 4, ...]
static uint32_t getRoundForBlockNumber(const AltChainParams& chainParams,
                                       const PopRewardsParams& rewardParams,
                                       uint32_t height) {
  if (height % chainParams.getKeystoneInterval() == 0) {
    return rewardParams.keystoneRound();
  }

  if (rewardParams.payoutRounds() <= 1) {
    return 0;
  }

  assert(height > 0);
  uint32_t round = (height % chainParams.getKeystoneInterval()) %
                   (rewardParams.payoutRounds() - 1);
  return round;
}

static bool isKeystoneRound(const PopRewardsParams& rewardParams,
                            uint32_t payoutRound) {
  return payoutRound == rewardParams.keystoneRound();
}

// test if block is placed inside payout round following keystone
static bool isFirstRoundAfterKeystone(const AltChainParams& chainParams,
                                      const PopRewardsParams& rewardParams,
                                      uint32_t height) {
  uint32_t blockAfterKeystone = height % chainParams.getKeystoneInterval();
  if (rewardParams.payoutRounds() == 0) return true;
  if (blockAfterKeystone / rewardParams.payoutRounds() == 0) return true;
  return false;
}

static PopRewardsBigDecimal getRoundRatio(const PopRewardsParams& rewardParams,
                                          uint32_t payoutRound) {
  return rewardParams.roundRatios()[payoutRound];
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
static PopRewardsBigDecimal calculateRewardWithSlope(
    const PopRewardsParams& rewardParams,
    PopRewardsBigDecimal score,
    uint32_t payoutRound) {
  PopRewardsBigDecimal slope = getRoundSlope(rewardParams, payoutRound);
  PopRewardsBigDecimal roundRatio = getRoundRatio(rewardParams, payoutRound);
  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();

  assert(score >= curveParams.startOfSlope());

  PopRewardsBigDecimal scoreDecrease =
      slope * (score - curveParams.startOfSlope());
  PopRewardsBigDecimal maxScoreDecrease = PopRewardsBigDecimal(1.0);
  if (scoreDecrease > maxScoreDecrease) {
    scoreDecrease = maxScoreDecrease;
  }

  // (1 - slope * (score - START_OF_DECREASING_LINE_REWARD)) * roundRatio *
  // score
  return (maxScoreDecrease - scoreDecrease) * roundRatio * score;
}

static PopRewardsBigDecimal calculateTotalPopBlockReward(
    const AltChainParams& chainParams,
    const PopRewardsParams& rewardParams,
    uint32_t height,
    PopRewardsBigDecimal difficulty,
    PopRewardsBigDecimal score) {
  if (score == PopRewardsBigDecimal(0.0)) {
    return PopRewardsBigDecimal(0.0);
  }

  // Minimum difficulty
  if (difficulty < PopRewardsBigDecimal(1.0)) {
    difficulty = PopRewardsBigDecimal(1.0);
  }

  uint32_t payoutRound =
      getRoundForBlockNumber(chainParams, rewardParams, height);

  PopRewardsBigDecimal scoreToDifficulty = score / difficulty;

  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();

  // No use of penalty multiplier, this payout occurs on the flat part of the
  // payout curve
  if (scoreToDifficulty <= curveParams.startOfSlope()) {
    PopRewardsBigDecimal roundRatio = getRoundRatio(rewardParams, payoutRound);

    // now we apply the current round multiplier to the score
    return scoreToDifficulty * roundRatio;
  }

  PopRewardsBigDecimal maxScoreThreshold =
      getMaxScoreThreshold(rewardParams, payoutRound);
  if (scoreToDifficulty > maxScoreThreshold) {
    scoreToDifficulty = maxScoreThreshold;
  }

  // Note that this reward per point is not the true rewardWithSlope if the
  // score to difficulty ratio is greater than the max reward threshold. Past
  // the max reward threshold, the block reward ceases to grow, but is split
  // amongst a larger number of participants.
  PopRewardsBigDecimal rewardWithSlope =
      calculateRewardWithSlope(rewardParams, scoreToDifficulty, payoutRound);

  return rewardWithSlope;
}

// we calculate the reward for a given block
PopRewardsBigDecimal PopRewardsCalculator::calculatePopRewardForBlock(
    const AltChainParams& chainParams,
    const PopRewardsParams& rewardParams,
    uint32_t height,
    PopRewardsBigDecimal scoreForThisBlock,
    PopRewardsBigDecimal difficulty) {
  if (scoreForThisBlock == PopRewardsBigDecimal(0.0)) {
    return PopRewardsBigDecimal(0.0);
  }

  // Special case for the first ROUND 3 after keystone - do not adjust for score
  // to difficulty ratio
  uint32_t roundNumber =
      getRoundForBlockNumber(chainParams, rewardParams, height);
  if (rewardParams.flatScoreRoundUse() &&
      roundNumber == rewardParams.flatScoreRound() &&
      isFirstRoundAfterKeystone(chainParams, rewardParams, height)) {
    return calculateTotalPopBlockReward(chainParams,
                                        rewardParams,
                                        height,
                                        PopRewardsBigDecimal(1.0),
                                        PopRewardsBigDecimal(1.0));
  }

  return calculateTotalPopBlockReward(
      chainParams, rewardParams, height, difficulty, scoreForThisBlock);
}

}  // namespace altintegration
