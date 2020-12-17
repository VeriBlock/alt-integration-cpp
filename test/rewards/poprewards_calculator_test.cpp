// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/rewards/poprewards_calculator_default.hpp"
#include "util/test_utils.hpp"

using namespace altintegration;

struct RewardsCalculatorTestFixture : ::testing::Test {
  AltChainParamsTest chainParams{};
  PopRewardsCalculatorDefault rewardsCalculator =
      PopRewardsCalculatorDefault(chainParams);
  PopRewardsBigDecimal defaultScore = 1.0;
  PopRewardsBigDecimal defaultDifficulty = 1.0;

  RewardsCalculatorTestFixture() {}
};

TEST_F(RewardsCalculatorTestFixture, basicReward_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  auto blockReward = rewardsCalculator.calculateBlockReward(
      height, defaultScore, defaultDifficulty);
  auto minerReward = rewardsCalculator.calculateMinerReward(
      0, defaultScore, blockReward);
  ASSERT_TRUE(minerReward > 0.0);
  ASSERT_EQ(minerReward, chainParams.getPayoutParams().roundRatios()[height]);

  // score < 1.0 is on the flat reward rate
  PopRewardsBigDecimal halfScore = defaultScore / 2.0;
  auto blockReward2 = rewardsCalculator.calculateBlockReward(
      height, halfScore, defaultDifficulty);
  auto minerReward2 =
      rewardsCalculator.calculateMinerReward(0, halfScore, blockReward2);
  ASSERT_TRUE(minerReward2 > 0.0);
  ASSERT_EQ(minerReward, minerReward2);

  // when score is higher than difficulty we begin to gradually decrease the
  // reward
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;
  auto blockReward3 = rewardsCalculator.calculateBlockReward(
      height, doubleScore, defaultDifficulty);
  auto minerReward3 = rewardsCalculator.calculateMinerReward(0, doubleScore, blockReward3);

  ASSERT_TRUE(minerReward3 > 0.0);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(minerReward, minerReward3);
  ASSERT_GT(blockReward3, blockReward);

  // we limit the reward to 200% threshold (for normal blocks). Let's check
  // this.
  PopRewardsBigDecimal doublePlusScore = defaultScore * 2.1;
  auto blockReward4 = rewardsCalculator.calculateBlockReward(
      height, doublePlusScore, defaultDifficulty);
  auto minerReward4 = rewardsCalculator.calculateMinerReward(0, doublePlusScore, blockReward4);
  ASSERT_GT(minerReward3, minerReward4);
  ASSERT_EQ(blockReward3, blockReward4);

  // test the keystone highest reward
  // assume three endorsements having 3.0 score in total
  auto blockReward5 = rewardsCalculator.calculateBlockReward(
      20, defaultScore * 3.0, defaultDifficulty);
  ASSERT_GT(blockReward5, blockReward4);

  // multiple endorsements may increase the score to 3.0, keystone block ratio
  // is 1.7 (due to reward curve) so the payout is 3.0 * 1.7 = 5.1
  ASSERT_NEAR((double)blockReward5.value.getLow64() / PopRewardsBigDecimal::decimals, 5.1, 0.1);

  // test over the highest reward
  auto blockReward6 = rewardsCalculator.calculateBlockReward(
      20, defaultScore * 4.0, defaultDifficulty);
  // we see that the reward is no longer growing
  ASSERT_EQ(blockReward6, blockReward5);

  auto blockReward7 = rewardsCalculator.calculateBlockReward(
      height, defaultScore, defaultDifficulty * 2.0);
  auto minerReward7 =
      rewardsCalculator.calculateMinerReward(0, defaultScore, blockReward7);
  ASSERT_TRUE(minerReward7 > 0.0);
  ASSERT_NEAR(
      (double)blockReward7.value.getLow64() / PopRewardsBigDecimal::decimals,
      0.5,
      0.1);
}

TEST_F(RewardsCalculatorTestFixture, specialReward_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  // let's start with hardcoded difficulty
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;
  auto blockReward = rewardsCalculator.calculateBlockReward(
      height, defaultScore, defaultDifficulty);
  auto minerReward1 = rewardsCalculator.calculateMinerReward(0, defaultScore, blockReward);
  auto blockReward2 = rewardsCalculator.calculateBlockReward(
      height, doubleScore, defaultDifficulty);
  auto minerReward2 = rewardsCalculator.calculateMinerReward(0, doubleScore, blockReward2);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(blockReward2, blockReward);
  ASSERT_GT(minerReward1, minerReward2);

  blockReward = rewardsCalculator.calculateBlockReward(
      height + 1, defaultScore, defaultDifficulty);
  auto minerReward3 = rewardsCalculator.calculateMinerReward(
      0, defaultScore, blockReward);
  blockReward2 = rewardsCalculator.calculateBlockReward(
      height + 1, doubleScore, defaultDifficulty);
  auto minerReward4 = rewardsCalculator.calculateMinerReward(
      0, doubleScore, blockReward2);

  // round 2 special case - any score has the same reward
  ASSERT_EQ(blockReward, blockReward2);
  ASSERT_GT(minerReward3, minerReward4);

  // now let's see how the keystone block is being rewarded
  blockReward = rewardsCalculator.calculateBlockReward(
      chainParams.getKeystoneInterval(), defaultScore, defaultDifficulty);
  auto minerRewardKeystone1 = rewardsCalculator.calculateMinerReward(
      0, defaultScore, blockReward);
  blockReward = rewardsCalculator.calculateBlockReward(
      chainParams.getKeystoneInterval(), doubleScore, defaultDifficulty);
  auto minerRewardKeystone2 = rewardsCalculator.calculateMinerReward(
      0, doubleScore, blockReward);
  ASSERT_GT(minerRewardKeystone1, minerRewardKeystone2);

  // we see that even when cut down the keystone reward is higher than any
  // normal reward from rounds 0-2
  ASSERT_GT(minerRewardKeystone2, minerReward1);
  ASSERT_GT(minerRewardKeystone2, minerReward2);
  ASSERT_GT(minerRewardKeystone2, minerReward3);
  ASSERT_GT(minerRewardKeystone2, minerReward4);
}
