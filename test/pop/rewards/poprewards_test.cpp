// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/rewards/default_poprewards_calculator.hpp>

#include "pop/util/pop_test_fixture.hpp"

using namespace altintegration;

struct RewardsTestFixture : public testing::TestWithParam<int>,
                            public PopTestFixture {
  BlockIndex<BtcBlock>* btctip;
  BlockIndex<VbkBlock>* vbktip;
  std::vector<AltBlock> altchain;
  std::shared_ptr<DefaultPopRewardsCalculator> sampleCalculator;

  ValidationState state;
  PopRewardsBigDecimal defaultScore = 1.0;
  PopRewardsBigDecimal defaultDifficulty = 1.0;

  RewardsTestFixture() {
    // hardcode keystone interval since reward algorithm is dependent
    // on it and is hard to recalculate for test
    altparam.mKeystoneInterval = 5;
    btctip = popminer->mineBtcBlocks(10);
    vbktip = popminer->mineVbkBlocks(10);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(10, altchain);

    sampleCalculator = std::make_shared<DefaultPopRewardsCalculator>(alttree);

    EXPECT_EQ(altchain.size(), 11);
    EXPECT_EQ(altchain.at(altchain.size() - 1).height, 10);
  }

  void endorseBlocks(const std::vector<AltBlock>& endorsedBlocks,
                     size_t endorsements) {
    size_t chainSize = altchain.size();
    std::vector<VbkTx> popTxs{};
    for (size_t i = 0; i < endorsements; i++) {
      for (const auto& b : endorsedBlocks) {
        uint256 stateRoot = generateRandomBytesVector(32);
        auto pubdata = generatePublicationData(b, stateRoot);
        VbkTx tx = popminer->createVbkTxEndorsingAltBlock(pubdata);
        popTxs.push_back(tx);
      }
    }
    auto* block = popminer->mineVbkBlocks(1, popTxs);

    PopData popData;
    for (const auto& popTx : popTxs) {
      popData.atvs.push_back(popminer->createATV(block->getHeader(), popTx));
    }
    fillVbkContext(popData.context,
                   alttree.vbk().getBestChain().tip()->getHash(),
                   popminer->vbk());

    auto* altTip = alttree.getBestChain().tip();
    auto nextBlock = generateNextBlock(altTip->getHeader());
    altchain.push_back(nextBlock);
    EXPECT_TRUE(alttree.acceptBlockHeader(nextBlock, state));
    ASSERT_TRUE(validatePayloads(nextBlock.getHash(), popData))
        << state.toString();
    ASSERT_TRUE(state.IsValid());
    EXPECT_EQ(altchain.size(), chainSize + 1);
  }

  void endorseBlock(AltBlock& endorsedBlock, size_t endorsements) {
    endorseBlocks({endorsedBlock}, endorsements);
  }

  void endorseForRewardLastBlock(size_t endorsements) {
    auto& endorsedBlock = altchain.back();
    endorseBlock(endorsedBlock, endorsements);
    // mine rewardSettlementInterval blocks + 1 - endorsed block - endorsement
    // block
    mineAltBlocks(altparam.getEndorsementSettlementInterval() - 1 - 1,
                  altchain);
  }

  void reorg(size_t blocks) {
    ASSERT_TRUE(altchain.size() >= blocks);
    altchain.resize(altchain.size() - blocks);
    ASSERT_TRUE(alttree.setState(altchain.back().getHash(), state));
    ASSERT_TRUE(state.IsValid());
  }
};

TEST_F(RewardsTestFixture, basicCalculator_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  auto blockReward = sampleCalculator->calculateBlockReward(
      height, defaultScore, defaultDifficulty);
  auto minerReward =
      sampleCalculator->calculateMinerReward(0, defaultScore, blockReward);
  ASSERT_TRUE(minerReward > 0.0);
  ASSERT_EQ(minerReward, altparam.getPayoutParams().roundRatios()[height]);

  // score < 1.0 is on the flat reward rate
  PopRewardsBigDecimal halfScore = defaultScore / 2.0;
  auto blockReward2 = sampleCalculator->calculateBlockReward(
      height, halfScore, defaultDifficulty);
  auto minerReward2 =
      sampleCalculator->calculateMinerReward(0, halfScore, blockReward2);
  ASSERT_TRUE(minerReward2 > 0.0);
  ASSERT_EQ(minerReward, minerReward2);

  // when score is higher than difficulty we begin to gradually decrease the
  // reward
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;
  auto blockReward3 = sampleCalculator->calculateBlockReward(
      height, doubleScore, defaultDifficulty);
  auto minerReward3 =
      sampleCalculator->calculateMinerReward(0, doubleScore, blockReward3);

  ASSERT_TRUE(minerReward3 > 0.0);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(minerReward, minerReward3);
  ASSERT_GT(blockReward3, blockReward);

  // we limit the reward to 200% threshold (for normal blocks). Let's check
  // this.
  PopRewardsBigDecimal doublePlusScore = defaultScore * 2.1;
  auto blockReward4 = sampleCalculator->calculateBlockReward(
      height, doublePlusScore, defaultDifficulty);
  auto minerReward4 =
      sampleCalculator->calculateMinerReward(0, doublePlusScore, blockReward4);
  ASSERT_GT(minerReward3, minerReward4);
  ASSERT_EQ(blockReward3, blockReward4);

  // test the keystone highest reward
  // assume three endorsements having 3.0 score in total
  auto blockReward5 = sampleCalculator->calculateBlockReward(
      20, defaultScore * 3.0, defaultDifficulty);
  ASSERT_GT(blockReward5, blockReward4);

  // multiple endorsements may increase the score to 3.0, keystone block ratio
  // is 1.7 (due to reward curve) so the payout is 3.0 * 1.7 = 5.1
  ASSERT_NEAR(
      (double)blockReward5.value.getLow64() / PopRewardsBigDecimal::decimals,
      5.1,
      0.1);

  // test over the highest reward
  auto blockReward6 = sampleCalculator->calculateBlockReward(
      20, defaultScore * 4.0, defaultDifficulty);
  // we see that the reward is no longer growing
  ASSERT_EQ(blockReward6, blockReward5);

  auto blockReward7 = sampleCalculator->calculateBlockReward(
      height, defaultScore, defaultDifficulty * 2.0);
  auto minerReward7 =
      sampleCalculator->calculateMinerReward(0, defaultScore, blockReward7);
  ASSERT_TRUE(minerReward7 > 0.0);
  ASSERT_NEAR(
      (double)blockReward7.value.getLow64() / PopRewardsBigDecimal::decimals,
      0.5,
      0.1);
}

TEST_F(RewardsTestFixture, specialReward_test) {
  // blockNumber is used to detect current round only. Let's start with round 1.
  uint32_t height = 1;

  // let's start with hardcoded difficulty
  PopRewardsBigDecimal doubleScore = defaultScore * 2.0;
  auto blockReward = sampleCalculator->calculateBlockReward(
      height, defaultScore, defaultDifficulty);
  auto minerReward1 =
      sampleCalculator->calculateMinerReward(0, defaultScore, blockReward);
  auto blockReward2 = sampleCalculator->calculateBlockReward(
      height, doubleScore, defaultDifficulty);
  auto minerReward2 =
      sampleCalculator->calculateMinerReward(0, doubleScore, blockReward2);
  // single miner reward is lower due to decreasing payout after 1.0 score
  // but 2.0 score means there are more miners hence higher total reward
  ASSERT_GT(blockReward2, blockReward);
  ASSERT_GT(minerReward1, minerReward2);

  blockReward = sampleCalculator->calculateBlockReward(
      height + 1, defaultScore, defaultDifficulty);
  auto minerReward3 =
      sampleCalculator->calculateMinerReward(0, defaultScore, blockReward);
  blockReward2 = sampleCalculator->calculateBlockReward(
      height + 1, doubleScore, defaultDifficulty);
  auto minerReward4 =
      sampleCalculator->calculateMinerReward(0, doubleScore, blockReward2);

  // round 2 special case - any score has the same reward
  ASSERT_EQ(blockReward, blockReward2);
  ASSERT_GT(minerReward3, minerReward4);

  // now let's see how the keystone block is being rewarded
  blockReward = sampleCalculator->calculateBlockReward(
      altparam.getKeystoneInterval(), defaultScore, defaultDifficulty);
  auto minerRewardKeystone1 =
      sampleCalculator->calculateMinerReward(0, defaultScore, blockReward);
  blockReward = sampleCalculator->calculateBlockReward(
      altparam.getKeystoneInterval(), doubleScore, defaultDifficulty);
  auto minerRewardKeystone2 =
      sampleCalculator->calculateMinerReward(0, doubleScore, blockReward);
  ASSERT_GT(minerRewardKeystone1, minerRewardKeystone2);

  // we see that even when cut down the keystone reward is higher than any
  // normal reward from rounds 0-2
  ASSERT_GT(minerRewardKeystone2, minerReward1);
  ASSERT_GT(minerRewardKeystone2, minerReward2);
  ASSERT_GT(minerRewardKeystone2, minerReward3);
  ASSERT_GT(minerRewardKeystone2, minerReward4);
}

TEST_F(RewardsTestFixture, basicReward_test) {
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(1);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_TRUE(payouts.size());

  auto payoutBlockRound =
      sampleCalculator->getRoundForBlockNumber(endorsedBlock.height);
  ASSERT_EQ(payouts.payouts.begin()->second,
            (int64_t)PopRewardsBigDecimal::decimals *
                altparam.getPayoutParams().roundRatios()[payoutBlockRound]);
  // keystone payout is 3 reward points
  ASSERT_NEAR(((double)payouts.payouts.begin()->second) /
                  PopRewardsBigDecimal::decimals,
              3,
              0.01);
}

TEST_F(RewardsTestFixture, largeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(30);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().keystoneRound());
  // and total miners' reward is 5.1 reward points
  ASSERT_NEAR(((double)payouts.payouts.begin()->second) /
                  PopRewardsBigDecimal::decimals,
              5.1,
              0.1);
}

TEST_F(RewardsTestFixture, hugeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(100);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().keystoneRound());
  // and total miners' reward is 5.1 reward points
  ASSERT_NEAR(((double)payouts.payouts.begin()->second) /
                  PopRewardsBigDecimal::decimals,
              5.1,
              0.1);
}

TEST_F(RewardsTestFixture, largeFlatReward_test) {
  mineAltBlocks(2, altchain);
  // ALT has genesis + 12 blocks
  EXPECT_EQ(altchain.size(), 13);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 12);

  // endorse ALT block, at height 12
  AltBlock endorsedBlock = altchain[12];
  endorseForRewardLastBlock(30);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().flatScoreRound());
  // and total miners' reward is 1.07 reward points
  ASSERT_NEAR(((double)payouts.payouts.begin()->second) /
                  PopRewardsBigDecimal::decimals,
              1.07,
              0.1);
}

TEST_F(RewardsTestFixture, hugeFlatReward_test) {
  mineAltBlocks(2, altchain);
  // ALT has genesis + 12 blocks
  EXPECT_EQ(altchain.size(), 13);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 12);

  // endorse ALT block, at height 12
  AltBlock endorsedBlock = altchain[12];
  endorseForRewardLastBlock(100);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().flatScoreRound());
  // and total miners' reward is 1.07 reward points
  ASSERT_NEAR(((double)payouts.payouts.begin()->second) /
                  PopRewardsBigDecimal::decimals,
              1.07,
              0.1);
}

TEST_F(RewardsTestFixture, basicCacheReward_test) {
  // mine 90 blocks and have 101 total
  mineAltBlocks(90, altchain, true);

  AltBlock endorsedBlock = altchain[95];
  endorseBlock(endorsedBlock, 1);
  auto* endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());

  EXPECT_EQ(altchain.back().height, 101);
  EXPECT_EQ(sampleCalculator->scoreFromEndorsements(*endorsedIndex), 1.0);
  mineAltBlocks(altparam.getPayoutParams().getPopPayoutDelay() - (101 - 95) - 1,
                altchain,
                true);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);

  PopPayouts payoutsUncached = {};
  ASSERT_TRUE(sampleCalculator->calculatePayouts(
      *endorsedIndex, payoutsUncached, state));
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.payouts.begin()->second,
            payouts.payouts.begin()->second);

  // generate new fork with the new altPayloads
  reorg(5);
  endorseBlock(endorsedBlock, 1);
  endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());

  // after reorg the score has changed
  EXPECT_FALSE(sampleCalculator->scoreFromEndorsements(*endorsedIndex) == 1.0);

  mineAltBlocks(4, altchain, true);
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  ASSERT_TRUE(sampleCalculator->calculatePayouts(
      *endorsedIndex, payoutsUncached, state));

  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.payouts.begin()->second,
            payouts.payouts.begin()->second);
}

static AltChainParamsRegTest altparam1{};
INSTANTIATE_TEST_SUITE_P(
    rewardsCacheRegression,
    RewardsTestFixture,
    testing::Range(1, (int)altparam1.getEndorsementSettlementInterval()));

TEST_P(RewardsTestFixture, continuousReorgsCacheReward_test) {
  int depth = GetParam();

  // mine 90 blocks and have 101 total
  mineAltBlocks(90, altchain, true);

  AltBlock endorsedBlock = altchain[95];
  AltBlock endorsedPrevBlock = altchain[94];

  endorseBlock(endorsedBlock, 1);
  auto* endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  auto* endorsedPrevIndex = alttree.getBlockIndex(endorsedPrevBlock.getHash());

  ASSERT_EQ(endorsedIndex->getEndorsedBy().size(), 1);
  ASSERT_EQ(endorsedPrevIndex->getEndorsedBy().size(), 0);

  EXPECT_EQ(altchain.back().height, 101);
  EXPECT_EQ(sampleCalculator->scoreFromEndorsements(*endorsedIndex), 1.0);
  mineAltBlocks(altparam.getPayoutParams().getPopPayoutDelay() - (101 - 95) - 1,
                altchain,
                true);

  PopPayouts payouts = {};
  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);

  PopPayouts payoutsUncached = {};
  ASSERT_TRUE(sampleCalculator->calculatePayouts(
      *endorsedIndex, payoutsUncached, state));
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.payouts.begin()->second,
            payouts.payouts.begin()->second);

  // generate new fork with the new altPayloads
  reorg(depth);

  // make 100 endorsements since we want to raise the average difficulty
  // over 1.0

  if (depth == 1) {
    // for depth = 1 endorsed previous block is before the endorsement
    // settlement do not try to endorse it so validation should not fail

    endorseBlock(endorsedBlock, 100);
  } else {
    endorseBlocks({endorsedPrevBlock, endorsedBlock}, 100);
  }

  endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  endorsedPrevIndex = alttree.getBlockIndex(endorsedPrevBlock.getHash());

  EXPECT_FALSE(sampleCalculator->scoreFromEndorsements(*endorsedIndex) == 1.0);

  mineAltBlocks(depth - 1, altchain, true);

  // current block height is 101 + 50 -1 - 101 + 95 + 1 = 145
  // reorgs longer than 44 blocks erase initial endorsement
  // therefore we only have 100 endorsements left
  if (depth < 44) {
    ASSERT_EQ(endorsedIndex->getEndorsedBy().size(), 101);
  } else {
    ASSERT_EQ(endorsedIndex->getEndorsedBy().size(), 100);
  }

  if (depth == 1) {
    // for depth = 1 endorsed block is before the endorsement settlement
    // interval therefore endorsedBy is not changed
    ASSERT_EQ(endorsedPrevIndex->getEndorsedBy().size(), 0);
  } else {
    ASSERT_EQ(endorsedPrevIndex->getEndorsedBy().size(), 100);
  }

  ASSERT_TRUE(sampleCalculator->getPopPayout(
      altchain.back().getHash(), payouts, state));
  ASSERT_EQ(payouts.size(), 1);
  ASSERT_TRUE(sampleCalculator->calculatePayouts(
      *endorsedIndex, payoutsUncached, state));

  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.payouts.begin()->second,
            payouts.payouts.begin()->second);
}

struct TestCase {
  uint32_t height;
  double score;
  double difficulty;
  double reward;
};

struct CalculatorTableFixture : public testing::TestWithParam<TestCase>,
                                public PopTestFixture {
  std::shared_ptr<DefaultPopRewardsCalculator> sampleCalculator;
  ValidationState state;

  CalculatorTableFixture() {
    sampleCalculator = std::make_shared<DefaultPopRewardsCalculator>(alttree);
  }
};

static const double POP_REWARD_MULTIPLIER = 20.0;

static std::vector<TestCase> calculatorTable_cases = {
    {3, 1.0, 1.0, 19.4},    {3, 2.0, 1.0, 31.04},   {3, 5.0, 1.0, 31.04},
    {3, 10.0, 1.0, 31.04},  {3, 25.0, 1.0, 31.04},  {3, 1.0, 2.0, 9.7},
    {3, 2.0, 2.0, 19.4},    {3, 5.0, 2.0, 31.04},   {3, 10.0, 2.0, 31.04},
    {3, 25.0, 2.0, 31.04},  {3, 1.0, 5.0, 3.88},    {3, 2.0, 5.0, 7.76},
    {3, 5.0, 5.0, 19.4},    {3, 10.0, 5.0, 31.04},  {3, 25.0, 5.0, 31.04},
    {3, 1.0, 10.0, 1.94},   {3, 2.0, 10.0, 3.88},   {3, 5.0, 10.0, 9.7},
    {3, 10.0, 10.0, 19.4},  {3, 25.0, 10.0, 31.04}, {3, 1.0, 25.0, 0.775},
    {3, 2.0, 25.0, 1.5525}, {3, 5.0, 25.0, 3.88},   {3, 10.0, 25.0, 7.76},
    {3, 25.0, 25.0, 19.4},  {4, 1.0, 1.0, 20.6},    {4, 2.0, 1.0, 32.96},
    {4, 5.0, 1.0, 32.96},   {4, 10.0, 1.0, 32.96},  {4, 25.0, 1.0, 32.96},
    {4, 1.0, 2.0, 10.3},    {4, 2.0, 2.0, 20.6},    {4, 5.0, 2.0, 32.96},
    {4, 10.0, 2.0, 32.96},  {4, 25.0, 2.0, 32.96},  {4, 1.0, 5.0, 4.12},
    {4, 2.0, 5.0, 8.24},    {4, 5.0, 5.0, 20.6},    {4, 10.0, 5.0, 32.96},
    {4, 25.0, 5.0, 32.96},  {4, 1.0, 10.0, 2.06},   {4, 2.0, 10.0, 4.12},
    {4, 5.0, 10.0, 10.3},   {4, 10.0, 10.0, 20.6},  {4, 25.0, 10.0, 32.96},
    {4, 1.0, 25.0, 0.825},  {4, 2.0, 25.0, 1.6475}, {4, 5.0, 25.0, 4.12},
    {4, 10.0, 25.0, 8.24},  {4, 25.0, 25.0, 20.6}};

TEST_P(CalculatorTableFixture, calculatorTable_test) {
  auto value = GetParam();
  const auto reward = sampleCalculator->calculateBlockReward(
      value.height, value.score, value.difficulty);
  ASSERT_NEAR((double)reward.value.getLow64() / PopRewardsBigDecimal::decimals *
                  POP_REWARD_MULTIPLIER,
              value.reward,
              0.01);
}

INSTANTIATE_TEST_SUITE_P(calculatorTableRegression,
                         CalculatorTableFixture,
                         testing::ValuesIn(calculatorTable_cases));
