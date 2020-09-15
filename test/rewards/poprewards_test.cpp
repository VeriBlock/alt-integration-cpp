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

    // mine rewardSettlementInterval blocks + 1 - endorsed block - endorsement block
    mineAltBlocks(altparam.getEndorsementSettlementInterval() - 1, altchain);
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

TEST_F(RewardsTestFixture, basicCacheReward_test) {
  /*mineAltBlocks(altparam.getRewardParams().difficultyAveragingInterval() + 1, altchain);
  AltBlock endorsedBlock = altchain.back();
  endorseLastBlock(10);

  auto payouts = alttree.getPopPayout(altchain.back().getHash());
  ASSERT_TRUE(payouts.size());

  auto payoutBlockRound =
      sampleCalculator->getRoundForBlockNumber(endorsedBlock.height);
  ASSERT_EQ(payouts.size(), 1);

  auto chain2 = altchain;
  chain2.resize(altchain.size() - 5);
  auto containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);
  auto
  PopData altPayloads3 = generateAltPayloads({tx}, vbkTip1->getHash());*/

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 100 blocks
  mineAltBlocks(100, chain, true);

  AltBlock endorsedBlock = chain[95];
  auto* endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // mine 5 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(5);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads1));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(chain.back().height, 101);
  EXPECT_EQ(sampleRewards->scoreFromEndorsements(*endorsedIndex),
            1.0);
  mineAltBlocks(altparam.getPopPayoutDelay() - (101 - 95), chain, true);

  auto payouts = alttree.getPopPayout(chain.back().getHash());
  ASSERT_EQ(payouts.size(), 1);

  auto payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_EQ(payoutsUncached.begin()->second, payouts.begin()->second);

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  vbkTip = alttree.vbk().getBestChain().tip();
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);

  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  PopData altPayloads3 = generateAltPayloads({tx2}, vbkTip->getHash());
  vbkTip = popminer->mineVbkBlocks(1);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads3));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  EXPECT_TRUE(state.IsValid());

  // after reorg the score has changed
  EXPECT_FALSE(sampleRewards->scoreFromEndorsements(*endorsedIndex) == 1.0);

  mineAltBlocks(4, chain2, true);
  payouts = alttree.getPopPayout(chain2.back().getHash());
  ASSERT_EQ(payouts.size(), 1);
  payoutsUncached = sampleRewards->calculatePayouts(*endorsedIndex);

  // we didn't call getPopPayout() for each subsequent block therefore
  // our cache is bad now
  ASSERT_EQ(payoutsUncached.size(), 1);
  ASSERT_NE(payoutsUncached.begin()->second, payouts.begin()->second);
}
