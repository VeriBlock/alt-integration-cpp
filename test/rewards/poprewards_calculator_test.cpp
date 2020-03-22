#include "veriblock/rewards/poprewards_calculator.hpp"

#include <gtest/gtest.h>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"

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

struct RewardsCalculatorTestFixture : ::testing::Test {
  PopRewardsCalculator rewardsCalculator{};
  AltChainParamsTest chainParams{};
  PopRewardsParams rewardParams{};
  PopRewardsBigDecimal defaultScore = PopRewardsBigDecimal(1.0);
  PopRewardsBigDecimal defaultDifficulty = PopRewardsBigDecimal(1.0);

  RewardsCalculatorTestFixture() {}
};

TEST_F(RewardsCalculatorTestFixture, basicReward_test) {
  // blockNumber is used to detect current round only. Let's start with ROUND_2.
  uint32_t height = 1;

  auto totalReward = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, defaultScore, defaultDifficulty);
  ASSERT_TRUE(totalReward > PopRewardsBigDecimal(0.0));

  PopRewardsBigDecimal halfScore = defaultScore / PopRewardsBigDecimal(2.0);

  auto totalReward2 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, halfScore, defaultDifficulty);
  ASSERT_TRUE(totalReward2 > PopRewardsBigDecimal(0.0));
  ASSERT_EQ(totalReward / PopRewardsBigDecimal(2.0), totalReward2);

  // when score is higher than difficulty we begin to gradually decrease the
  // reward
  PopRewardsBigDecimal doubleScore = defaultScore * PopRewardsBigDecimal(2.0);
  auto totalReward3 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, doubleScore, defaultDifficulty);

  ASSERT_TRUE(totalReward3 > PopRewardsBigDecimal(0.0));
  ASSERT_GT(totalReward * PopRewardsBigDecimal(2.0), totalReward3);
  ASSERT_GT(totalReward3, totalReward);

  // we limit the reward to 200% threshold (for normal blocks). Let's check
  // this.
  PopRewardsBigDecimal doublePlusScore =
      defaultScore * PopRewardsBigDecimal(2.1);
  auto totalReward4 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams, rewardParams, height, doublePlusScore, defaultDifficulty);
  ASSERT_EQ(totalReward3, totalReward4);

  // test the keystone highest reward
  auto totalReward5 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams,
      rewardParams,
      20,
      defaultScore * PopRewardsBigDecimal(3.0),
      defaultDifficulty);
  ASSERT_GT(totalReward5, totalReward4);

  // multiple endorsements may increase the score to 3.0, keystone block ratio
  // is 1.7 (due to reward curve) so the payout is 3.0 * 1.7 = 5.1 *
  // REWARD_PER_BLOCK
  ASSERT_GT(totalReward5, PopRewardsBigDecimal(5.0));
  ASSERT_LT(totalReward5, PopRewardsBigDecimal(6.0));

  // test over the highest reward
  auto totalReward6 = rewardsCalculator.calculatePopRewardForBlock(
      chainParams,
      rewardParams,
      20,
      defaultScore * PopRewardsBigDecimal(4.0),
      defaultDifficulty);
  // we see that the reward is no longer growing
  ASSERT_EQ(totalReward6, totalReward5);
}

TEST_F(RewardsCalculatorTestFixture, specialReward_test) {
  // blockNumber is used to detect current round only. Let's start with ROUND_2.
  uint32_t height = 1;

  // let's start with hardcoded difficulty
  PopRewardsBigDecimal halfScore = defaultScore / PopRewardsBigDecimal(2.0);

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
