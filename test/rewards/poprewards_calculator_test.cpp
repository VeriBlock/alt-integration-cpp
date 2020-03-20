#include <gtest/gtest.h>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getGenesisBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

struct RewardsTestFixture : ::testing::Test {
  PopRewardsCalculator rewardsCalculator{};
  AltChainParamsTest chainParams{};
  PopRewardsParams rewardParams{};
  ArithUint256 defaultScore = 1LL * rewardsDecimalsMult;
  ArithUint256 defaultDifficulty = 1LL * rewardsDecimalsMult;

  RewardsTestFixture() {}
};

TEST_F(RewardsTestFixture, basicReward_test) {
  // blockNumber is used to detect current round only. Let's start with ROUND_2.
  uint32_t height = 1;

  auto totalReward = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, defaultScore, defaultDifficulty);
  ASSERT_TRUE(totalReward > 0);

  ArithUint256 halfScore = defaultScore / 2;

  auto totalReward2 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, halfScore, defaultDifficulty);
  ASSERT_TRUE(totalReward2 > 0);
  ASSERT_EQ(totalReward / 2, totalReward2);

  // when score is higher than difficulty we begin to gradually decrease the
  // reward
  ArithUint256 doubleScore = defaultScore * 2;
  auto totalReward3 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, doubleScore, defaultDifficulty);

  ASSERT_TRUE(totalReward3 > 0);
  ASSERT_GT(totalReward * 2, totalReward3);
  ASSERT_GT(totalReward3, totalReward);

  // we limit the reward to 200% threshold (for normal blocks). Let's check
  // this.
  ArithUint256 doublePlusScore = (uint32_t)(2.1 * rewardsDecimalsMult);
  auto totalReward4 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, doublePlusScore, defaultDifficulty);
  ASSERT_EQ(totalReward3, totalReward4);

  // test the keystone highest reward
  auto totalReward5 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, 20, defaultScore * 3, defaultDifficulty);
  ASSERT_GT(totalReward5, totalReward4);

  // multiple endorsements may increase the score to 3.0, keystone block ratio
  // is 1.7 (due to reward curve) so the payout is 3.0 * 1.7 = 5.1 *
  // REWARD_PER_BLOCK
  uint64_t totalReward5Value = totalReward5.getLow64();
  ASSERT_GT(totalReward5Value, 5ULL * rewardsDecimalsMult);
  ASSERT_LT(totalReward5Value, 6ULL * rewardsDecimalsMult);

  // test over the highest reward
  auto totalReward6 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, 20, defaultScore * 4, defaultDifficulty);
  // we see that the reward is no longer growing
  ASSERT_EQ(totalReward6, totalReward5);
}

TEST_F(RewardsTestFixture, specialReward_test) {
  // blockNumber is used to detect current round only. Let's start with ROUND_2.
  uint32_t height = 1;

  // let's start with hardcoded difficulty
  ArithUint256 halfScore = defaultScore / 2;

  auto totalReward1 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, defaultScore, defaultDifficulty);
  auto totalReward2 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, halfScore, defaultDifficulty);
  ASSERT_GT(totalReward1, totalReward2);

  auto totalReward3 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height + 1, defaultScore, defaultDifficulty);
  auto totalReward4 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height + 1, halfScore, defaultDifficulty);

  // round 3 special case - any score has the same reward
  ASSERT_EQ(totalReward3, totalReward4);

  // now let's see how the keystone block is being rewarded
  auto totalRewardKeystone1 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams,
      rewardParams,
      chainParams.getKeystoneInterval(),
      defaultScore,
      defaultDifficulty);
  auto totalRewardKeystone2 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams,
      rewardParams,
      chainParams.getKeystoneInterval(),
      halfScore,
      defaultDifficulty);
  ASSERT_GT(totalRewardKeystone1, totalRewardKeystone2);

  // we see that even when cut in half the keystone reward is higher than any
  // normal reward from rounds 1-3
  ASSERT_GT(totalRewardKeystone2, totalReward1);
  ASSERT_GT(totalRewardKeystone2, totalReward2);
  ASSERT_GT(totalRewardKeystone2, totalReward3);
  ASSERT_GT(totalRewardKeystone2, totalReward4);
}
