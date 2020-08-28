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
      auto pubdata = generatePublicationData(endorsedBlock);
      pubdata.contextInfo.push_back((unsigned char)i);
      VbkTx tx = popminer->createVbkTxEndorsingAltBlock(pubdata);
      popTxs.push_back(tx);
    }
    auto atvs = popminer->applyATVs(popTxs, state);

    PopData popData;
    popData.atvs = atvs;
    fillVbkContext(
        popData.context, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

    auto* altTip = alttree.getBestChain().tip();
    auto nextBlock = generateNextBlock(altTip->getHeader());
    altchain.push_back(nextBlock);
    EXPECT_TRUE(alttree.acceptBlockHeader(nextBlock, state));
    ASSERT_TRUE(validatePayloads(nextBlock.getHash(), popData));
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
  // keystone payout is 3 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
      3,
      0.01);
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
  // and total miners' reward is 5.1 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
      5.1,
      0.1);
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
  endorseLastBlock(30);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().flatScoreRound());
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
  endorseLastBlock(100);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  // make sure we have calculations for the flat score round
  ASSERT_EQ(sampleCalculator->getRoundForBlockNumber(endorsedBlock.height),
            altparam.getRewardParams().flatScoreRound());
  // and total miners' reward is 1.07 reward points
  ASSERT_NEAR(
      ((double)payouts.begin()->second) / PopRewardsBigDecimal::decimals,
      1.07,
      0.1);
}
