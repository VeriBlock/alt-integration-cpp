// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct RewardsTestFixture : public testing::TestWithParam<int>,
                            public PopTestFixture {
  BlockIndex<BtcBlock>* btctip;
  BlockIndex<VbkBlock>* vbktip;
  std::vector<AltBlock> altchain;
  std::shared_ptr<PopRewardsCalculator> sampleCalculator;
  std::shared_ptr<PopRewards> sampleRewards;

  ValidationState state;

  RewardsTestFixture() {
    btctip = popminer->mineBtcBlocks(10);
    vbktip = popminer->mineVbkBlocks(10);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(10, altchain);

    sampleCalculator = std::make_shared<PopRewardsCalculator>(altparam);
    sampleRewards = std::make_shared<PopRewards>(altparam, alttree.vbk());

    EXPECT_EQ(altchain.size(), 11);
    EXPECT_EQ(altchain.at(altchain.size() - 1).height, 10);
  }

  void endorseBlocks(const std::vector<AltBlock>& endorsedBlocks,
                     size_t endorsements) {
    size_t chainSize = altchain.size();
    std::vector<VbkTx> popTxs{};
    for (size_t i = 0; i < endorsements; i++) {
      for (const auto& b : endorsedBlocks) {
        auto pubdata = generatePublicationData(b);
        pubdata.contextInfo.push_back((unsigned char)i);
        VbkTx tx = popminer->createVbkTxEndorsingAltBlock(pubdata);
        popTxs.push_back(tx);
      }
    }
    auto atvs = popminer->applyATVs(popTxs, state);

    PopData popData;
    popData.atvs = atvs;
    fillVbkContext(popData.context,
                   alttree.vbk().getBestChain().tip()->getHash(),
                   popminer->vbk());

    auto* altTip = alttree.getBestChain().tip();
    auto nextBlock = generateNextBlock(altTip->getHeader());
    altchain.push_back(nextBlock);
    EXPECT_TRUE(alttree.acceptBlockHeader(nextBlock, state));
    ASSERT_TRUE(validatePayloads(nextBlock.getHash(), popData));
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
    mineAltBlocks(altparam.getEndorsementSettlementInterval() - 1, altchain);
  }

  void reorg(size_t blocks) {
    ASSERT_TRUE(altchain.size() >= blocks);
    altchain.resize(altchain.size() - blocks);
    ASSERT_TRUE(alttree.setState(altchain.back().getHash(), state));
    ASSERT_TRUE(state.IsValid());
  }
};

TEST_F(RewardsTestFixture, basicReward_test) {
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(1);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_TRUE(payouts.size());

  auto payoutBlockRound =
      sampleCalculator->getRoundForBlockNumber(endorsedBlock.height);
  ASSERT_EQ(payouts.begin()->second,
            (int64_t)PopRewardsBigDecimal::decimals *
                altparam.getPayoutParams().roundRatios()[payoutBlockRound]);
  // keystone payout is 3 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
      3,
      0.01);
}

TEST_F(RewardsTestFixture, largeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(30);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().keystoneRound());
  // and total miners' reward is 5.1 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
      5.1,
      0.1);
}

TEST_F(RewardsTestFixture, hugeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseForRewardLastBlock(100);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().keystoneRound());
  // and total miners' reward is 5.1 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
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

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().flatScoreRound());
  // and total miners' reward is 1.07 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
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

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getPayoutParams().flatScoreRound());
  // and total miners' reward is 1.07 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
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
  EXPECT_EQ(sampleRewards->scoreFromEndorsements(*endorsedIndex), 1.0);
  mineAltBlocks(altparam.getPayoutParams().getPopPayoutDelay() - (101 - 95),
                altchain,
                true);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);

  auto payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.begin()->second, payouts.begin()->second);

  // generate new fork with the new altPayloads
  reorg(5);
  endorseBlock(endorsedBlock, 1);
  endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());

  // after reorg the score has changed
  EXPECT_FALSE(sampleRewards->scoreFromEndorsements(*endorsedIndex) == 1.0);

  mineAltBlocks(4, altchain, true);
  payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);

  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.begin()->second, payouts.begin()->second);
}

static AltChainParamsRegTest altparam1{};
INSTANTIATE_TEST_SUITE_P(
    rewardsCacheRegression,
    RewardsTestFixture,
    testing::Range(1, altparam1.getEndorsementSettlementInterval() + 1));

TEST_P(RewardsTestFixture, continuousReorgsCacheReward_test) {
  int depth = GetParam();

  // mine 90 blocks and have 101 total
  mineAltBlocks(90, altchain, true);

  AltBlock endorsedBlock = altchain[95];
  AltBlock endorsedPrevBlock = altchain[94];

  endorseBlock(endorsedBlock, 1);
  auto* endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  auto* endorsedPrevIndex = alttree.getBlockIndex(endorsedPrevBlock.getHash());

  ASSERT_EQ(endorsedIndex->endorsedBy.size(), 1);
  ASSERT_EQ(endorsedPrevIndex->endorsedBy.size(), 0);

  EXPECT_EQ(altchain.back().height, 101);
  EXPECT_EQ(sampleRewards->scoreFromEndorsements(*endorsedIndex), 1.0);
  mineAltBlocks(altparam.getPayoutParams().getPopPayoutDelay() - (101 - 95),
                altchain,
                true);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);

  auto payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.begin()->second, payouts.begin()->second);

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

  EXPECT_FALSE(sampleRewards->scoreFromEndorsements(*endorsedIndex) == 1.0);

  mineAltBlocks(depth - 1, altchain, true);

  // current block height is 101 + 50 - 101 + 95 + 1 = 146
  // reorgs longer than 45 blocks erase initial endorsement
  // therefore we only have 100 endorsements left
  if (depth < 45) {
    ASSERT_EQ(endorsedIndex->endorsedBy.size(), 101);
  } else {
    ASSERT_EQ(endorsedIndex->endorsedBy.size(), 100);
  }

  if (depth == 1) {
    // for depth = 1 endorsed block is before the endorsement settlement
    // interval therefore endorsedBy is not changed
    ASSERT_EQ(endorsedPrevIndex->endorsedBy.size(), 0);
  } else {
    ASSERT_EQ(endorsedPrevIndex->endorsedBy.size(), 100);
  }

  payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);

  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.begin()->second, payouts.begin()->second);
}
