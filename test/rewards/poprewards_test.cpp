// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct RewardsTestFixture : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btctip;
  BlockIndex<VbkBlock>* vbktip;
  std::vector<AltBlock> altchain;
  std::shared_ptr<PopRewardsCalculator> sampleCalculator;

  ValidationState state;

  RewardsTestFixture() {
    btctip = popminer->mineBtcBlocks(10);
    vbktip = popminer->mineVbkBlocks(10);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(10, altchain);

    sampleCalculator = std::make_shared<PopRewardsCalculator>(altparam);

    EXPECT_EQ(altchain.size(), 11);
    EXPECT_EQ(altchain.at(altchain.size() - 1).height, 10);
  }

  void endorseLastBlock(size_t endorsements) {
    size_t chainSize = altchain.size();
    AltBlock endorsedBlock = *altchain.rbegin();
    std::vector<VbkTx> popTxs{};
    for (size_t i = 0; i < endorsements; i++) {
      VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
          generatePublicationData(endorsedBlock));
      popTxs.push_back(tx);
    }
    AltBlock containingBlock = generateNextBlock(*altchain.rbegin());
    altchain.push_back(containingBlock);

    auto altPayloads1 =
        generateAltPayloads(popTxs, vbkparam.getGenesisBlock().getHash());

    EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
    ASSERT_TRUE(
        validatePayloads(containingBlock.getHash(), altPayloads1, state));
    ASSERT_TRUE(state.IsValid());
    EXPECT_EQ(altchain.size(), chainSize + 1);

    // mine rewardSettlementInterval blocks - endorsed block - endorsement block
    mineAltBlocks(altparam.getEndorsementSettlementInterval() - 2, altchain);
  }
};

TEST_F(RewardsTestFixture, basicReward_test) {
  AltBlock endorsedBlock = altchain[10];
  endorseLastBlock(1);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_TRUE(payouts.size());

  auto payoutBlockRound =
      sampleCalculator->getRoundForBlockNumber(endorsedBlock.height);
  ASSERT_EQ(payouts.begin()->second,
            (int64_t)PopRewardsBigDecimal::decimals *
                altparam.getRewardParams().roundRatios()[payoutBlockRound]);
}

TEST_F(RewardsTestFixture, largeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseLastBlock(30);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().keystoneRound());
  // and total miners' reward is 25 reward points
  ASSERT_GT(payouts.begin()->second, 24LL * PopRewardsBigDecimal::decimals);
  ASSERT_LT(payouts.begin()->second, 26LL * PopRewardsBigDecimal::decimals);
}

TEST_F(RewardsTestFixture, hugeKeystoneReward_test) {
  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  endorseLastBlock(100);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the keystone round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().keystoneRound());
  // and total miners' reward is 25 reward points
  ASSERT_GT(payouts.begin()->second, 24LL * PopRewardsBigDecimal::decimals);
  ASSERT_LT(payouts.begin()->second, 26LL * PopRewardsBigDecimal::decimals);
}

TEST_F(RewardsTestFixture, largeFlatReward_test) {
  mineAltBlocks(2, altchain);
  // ALT has genesis + 12 blocks
  EXPECT_EQ(altchain.size(), 13);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 12);

  // endorse ALT block, at height 12
  AltBlock endorsedBlock = altchain[12];
  endorseLastBlock(30);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().flatScoreRound());
  // and total miners' reward is 15 reward points
  ASSERT_GT(payouts.begin()->second, 15LL * PopRewardsBigDecimal::decimals);
  ASSERT_LT(payouts.begin()->second, 16LL * PopRewardsBigDecimal::decimals);
}

TEST_F(RewardsTestFixture, hugeFlatReward_test) {
  mineAltBlocks(2, altchain);
  // ALT has genesis + 12 blocks
  EXPECT_EQ(altchain.size(), 13);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 12);

  // endorse ALT block, at height 12
  AltBlock endorsedBlock = altchain[12];
  endorseLastBlock(100);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().flatScoreRound());
  // and total miners' reward is 15 reward points
  ASSERT_GT(payouts.begin()->second, 15LL * PopRewardsBigDecimal::decimals);
  ASSERT_LT(payouts.begin()->second, 16LL * PopRewardsBigDecimal::decimals);
}
