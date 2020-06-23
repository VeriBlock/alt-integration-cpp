// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }

  uint32_t getIdentifier() const noexcept override { return 0x7ec7; }

  uint32_t getMaxPopDataPerBlock() const noexcept override { return 50; }

  uint32_t getMaxPopDataWeight() const noexcept override { return 1000000; }

  uint32_t getSuperMaxPopDataWeight() const noexcept override {
    return 5 * getMaxPopDataWeight();
  }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    ReadStream stream(bytes);
    AltBlock altBlock = AltBlock::fromVbkEncoding(stream);
    return altBlock.getHash();
  }
};

struct RewardsCalculatorTestFixture : ::testing::Test {
  AltChainParamsTest chainParams{};
  PopRewardsCalculator rewardsCalculator = PopRewardsCalculator(chainParams);
  PopRewardsBigDecimal defaultScore = 1.0;
  PopRewardsBigDecimal defaultDifficulty = 1.0;

  RewardsCalculatorTestFixture() {}
};

TEST_F(RewardsCalculatorTestFixture, basicReward_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  auto minerReward = rewardsCalculator.calculateMinerReward(
      height, 0, defaultScore, defaultDifficulty);
  ASSERT_TRUE(minerReward > 0.0);
  ASSERT_EQ(minerReward, chainParams.getRewardParams().roundRatios()[height]);

  // score < 1.0 is on the flat reward rate
  PopRewardsBigDecimal halfScore = defaultScore / 2.0;
  auto minerReward2 = rewardsCalculator.calculateMinerReward(
      height, 0, halfScore, defaultDifficulty);
  ASSERT_TRUE(minerReward2 > 0.0);
  ASSERT_EQ(minerReward, minerReward2);

  // when score is higher than difficulty we begin to gradually decrease the
  // reward
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;
  auto minerReward3 = rewardsCalculator.calculateMinerReward(
      height, 0, doubleScore, defaultDifficulty);

  ASSERT_TRUE(minerReward3 > 0.0);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(minerReward, minerReward3);

  // we limit the reward to 200% threshold (for normal blocks). Let's check
  // this.
  PopRewardsBigDecimal doublePlusScore = defaultScore * 2.1;
  auto minerReward4 = rewardsCalculator.calculateMinerReward(
      height, 0, doublePlusScore, defaultDifficulty);
  ASSERT_EQ(minerReward3, minerReward4);

  // test the keystone highest reward
  // assume three endorsements having 3.0 score in total
  auto minerReward5 = rewardsCalculator.calculateMinerReward(
      20, 0, defaultScore * 3.0, defaultDifficulty);
  ASSERT_GT(minerReward5, minerReward4);

  // multiple endorsements may increase the score to 3.0, keystone block ratio
  // is 1.7 (due to reward curve) so the payout is 3.0 * 1.7 = 5.1
  ASSERT_GT(minerReward5 * 3.0, 5.0);
  ASSERT_LT(minerReward5 * 3.0, 6.0);

  // test over the highest reward
  auto minerReward6 = rewardsCalculator.calculateMinerReward(
      20, 0, defaultScore * 4.0, defaultDifficulty);
  // we see that the reward is no longer growing
  ASSERT_EQ(minerReward6, minerReward5);
}

TEST_F(RewardsCalculatorTestFixture, specialReward_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  // let's start with hardcoded difficulty
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;

  auto minerReward1 = rewardsCalculator.calculateMinerReward(
      height, 0, defaultScore, defaultDifficulty);
  auto minerReward2 = rewardsCalculator.calculateMinerReward(
      height, 0, doubleScore, defaultDifficulty);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(minerReward1, minerReward2);

  auto minerReward3 = rewardsCalculator.calculateMinerReward(
      height + 1, 0, defaultScore, defaultDifficulty);
  auto minerReward4 = rewardsCalculator.calculateMinerReward(
      height + 1, 0, doubleScore, defaultDifficulty);

  // round 2 special case - any score has the same reward
  ASSERT_EQ(minerReward3, minerReward4);

  // now let's see how the keystone block is being rewarded
  auto minerRewardKeystone1 = rewardsCalculator.calculateMinerReward(
      chainParams.getKeystoneInterval(), 0, defaultScore, defaultDifficulty);
  auto minerRewardKeystone2 = rewardsCalculator.calculateMinerReward(
      chainParams.getKeystoneInterval(), 0, doubleScore, defaultDifficulty);
  ASSERT_GT(minerRewardKeystone1, minerRewardKeystone2);

  // we see that even when cut down the keystone reward is higher than any
  // normal reward from rounds 0-2
  ASSERT_GT(minerRewardKeystone2, minerReward1);
  ASSERT_GT(minerRewardKeystone2, minerReward2);
  ASSERT_GT(minerRewardKeystone2, minerReward3);
  ASSERT_GT(minerRewardKeystone2, minerReward4);
}
