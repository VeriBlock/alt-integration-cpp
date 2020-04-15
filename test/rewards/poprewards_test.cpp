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
    btctip = popminer.mineBtcBlocks(10);
    vbktip = popminer.mineVbkBlocks(10);

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
  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());
  // ALT has 11 blocks + endorsement block
  EXPECT_EQ(altchain.size(), 12);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 11);

  ///TODO: mine additional blocks so endorsement block passes rewardSettlement

  auto payouts = alttree.getPopPayout(containingBlock.getHash());
  ASSERT_TRUE(payouts.size() > 0);
}
