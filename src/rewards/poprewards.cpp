#include <algorithm>
#include <cassert>
#include <vector>
#include <veriblock/rewards/poprewards.hpp>

namespace VeriBlock {

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
  uint32_t round = ((height - 1) % chainParams.getKeystoneInterval()) %
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

// resulting ratio has rewardsDecimalsMult precision
static uint32_t getRoundRatio(const PopRewardsParams& rewardParams,
                              uint32_t payoutRound) {
  return rewardParams.roundRatios()[payoutRound];
}

// resulting threshold has rewardsDecimalsMult precision
static uint32_t getMaxRewardThreshold(const PopRewardsParams& rewardParams,
                                      uint32_t payoutRound) {
  if (isKeystoneRound(rewardParams, payoutRound)) {
    return rewardParams.maxRewardThresholdKeystone();
  }
  return rewardParams.maxRewardThresholdNormal();
}

// slope is how the payout is decreased for each additional block score
static uint32_t getRoundSlope(const PopRewardsParams& rewardParams,
                              uint32_t payoutRound) {
  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();
  uint64_t aboveIntendedPayoutMultiplier =
      curveParams.aboveIntendedPayoutMultiplierNormal();
  uint64_t widthOfDecreasingLine = curveParams.widthOfDecreasingLineNormal();

  if (payoutRound == rewardParams.keystoneRound()) {
    aboveIntendedPayoutMultiplier =
        curveParams.aboveIntendedPayoutMultiplierKeystone();
    widthOfDecreasingLine = curveParams.widthOfDecreasingLineKeystone();
  }

  uint64_t maxSlope = 1LL * rewardsDecimalsMult;

  // we only support falling reward slope
  assert(aboveIntendedPayoutMultiplier <= maxSlope);

  if (widthOfDecreasingLine == 0) return 0;

  return
      uint32_t(
        // 1 - aboveIntendedPayoutMultiplier
        (maxSlope - aboveIntendedPayoutMultiplier) *
        // increase precision two times
        rewardsDecimalsMult /
        // have rewardsDecimalsMult precision in the end
        widthOfDecreasingLine);
}

// apply the reward curve to the score and subtract it from the current round
// multiplier
static ArithUint256 calculateRewardWithSlope(
    const PopRewardsParams& rewardParams,
                                         ArithUint256 score,
                                         uint32_t payoutRound) {
  ArithUint256 slope = getRoundSlope(rewardParams, payoutRound);
  uint32_t roundRatio = getRoundRatio(rewardParams, payoutRound);
  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();

  ArithUint256 scoreDecrease = slope *
                           (score - curveParams.startOfDecreasingLine()) /
                           rewardsDecimalsMult;

  uint64_t maxScoreDecrease = 1LL * rewardsDecimalsMult;

  if (scoreDecrease > maxScoreDecrease) {
    scoreDecrease = maxScoreDecrease;
  }

  // (1 - slope * (score - START_OF_DECREASING_LINE_REWARD)) * roundRatio
  return (maxScoreDecrease - scoreDecrease) * roundRatio / rewardsDecimalsMult;
}

static ArithUint256 calculateTotalPopBlockReward(
    const AltChainParams& chainParams,
    const PopRewardsParams& rewardParams,
    uint32_t height,
    ArithUint256 difficulty,
    ArithUint256 score) {
  if (score == 0) {
    return 0;
  }

  // Minimum difficulty
  if (difficulty < 1LL * rewardsDecimalsMult) {
    difficulty = 1LL * rewardsDecimalsMult;
  }

  uint32_t payoutRound =
      getRoundForBlockNumber(chainParams, rewardParams, height);

  ArithUint256 scoreToDifficulty =
      // increase precision two times before division
      score * rewardsDecimalsMult / difficulty;

  PopRewardsCurveParams curveParams = rewardParams.getCurveParams();

  // No use of penalty multiplier, this payout occurs on the flat part of the
  // payout curve
  if (scoreToDifficulty <= curveParams.startOfDecreasingLine()) {
    uint32_t roundRatio = getRoundRatio(rewardParams, payoutRound);

    // now we apply the score to our budget and apply the current round
    // multiplier
    return scoreToDifficulty * roundRatio / rewardsDecimalsMult;
  }

  uint32_t maxRewardThreshold = getMaxRewardThreshold(rewardParams, payoutRound);
  if (score > maxRewardThreshold) {
    score = maxRewardThreshold;
  }

  // Note that this reward per point is not the true rewardWithSlope if the
  // score to difficulty ratio is greater than the max reward threshold. Past
  // the max reward threshold, the block reward ceases to grow, but is split
  // amongst a larger number of participants.
  ArithUint256 rewardWithSlope =
      calculateRewardWithSlope(rewardParams, scoreToDifficulty, payoutRound);

  return rewardWithSlope;
}

// we calculate the reward for a given block
ArithUint256 PopRewards::calculatePopRewardForBlock(
    const AltChainParams& chainParams,
    const PopRewardsParams& rewardParams,
    uint32_t height,
    ArithUint256 scoreForThisBlock,
    ArithUint256 difficulty) {
  if (scoreForThisBlock == 0) {
    return 0;
  }

  // Special case for the first ROUND 3 after keystone - do not adjust for score
  // to difficulty ratio
  uint32_t roundNumber =
      getRoundForBlockNumber(chainParams, rewardParams, height);
  if (rewardParams.flatScoreRoundUse() &&
      roundNumber == rewardParams.flatScoreRound() &&
      isFirstRoundAfterKeystone(chainParams, rewardParams, height)) {
    return calculateTotalPopBlockReward(
        chainParams, rewardParams, height, 1, 1);
  }

  return calculateTotalPopBlockReward(
      chainParams, rewardParams, height, difficulty, scoreForThisBlock);
}

}  // namespace VeriBlock
