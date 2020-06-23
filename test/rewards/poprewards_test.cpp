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

  ValidationState state;

  RewardsTestFixture() {
    btctip = popminer->mineBtcBlocks(10);
    vbktip = popminer->mineVbkBlocks(10);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(10, altchain);
  }
};

TEST_F(RewardsTestFixture, basicReward_test) {
  // ALT has genesis + 10 blocks
  EXPECT_EQ(altchain.size(), 11);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 10);

  // endorse ALT block, at height 10
  AltBlock endorsedBlock = altchain[10];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, altPayloads1, state));
  EXPECT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());
  // ALT has 11 blocks + endorsement block
  EXPECT_EQ(altchain.size(), 12);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 11);

  // mine rewardSettlementInterval blocks - endorsed block - endorsement block
  mineAltBlocks(altparam.getEndorsementSettlementInterval() - 2, altchain);

  auto payouts = alttree.getPopPayout(altchain.back().getHash(), state);
  ASSERT_TRUE(payouts.size());

  PopRewardsCalculator sampleCalculator = PopRewardsCalculator(altparam);
  auto payoutBlockRound =
      sampleCalculator.getRoundForBlockNumber(endorsedBlock.height);
  ASSERT_EQ(payouts[tx.publicationData.payoutInfo],
            (int64_t)PopRewardsBigDecimal::decimals *
                altparam.getRewardParams().roundRatios()[payoutBlockRound]);
}
