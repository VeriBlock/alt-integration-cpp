// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct PopFrInvalidVbkChainTest : public ::testing::Test,
                                  public PopTestFixture {
  PopFrInvalidVbkChainTest() {}
};

TEST_F(PopFrInvalidVbkChainTest, SendInvalidVTBtoAlternativeVBKchain) {
  // prepare:
  // tipA is 40 blocks long
  auto *tipA = popminer->mineVbkBlocks(40);
  // tipB forks at 10, and has 28 more blocks. tipA is best chain
  auto *tipB = popminer->mineVbkBlocks(*tipA->getAncestor(10), 28);
  EXPECT_EQ(*popminer->vbk().getBestChain().tip(), *tipA);

  // next block in B endorses block number 25 in its chain twice, and
  // containing is block 39
  auto vbkpoptx1 = popminer->endorseVbkBlock(
      *tipB->getAncestor(25)->header, getLastKnownBtcBlock(), state);
  auto vbkpoptx2 = popminer->endorseVbkBlock(
      *tipB->getAncestor(25)->header, getLastKnownBtcBlock(), state);
  popminer->vbkmempool.push_back(vbkpoptx1);
  popminer->vbkmempool.push_back(vbkpoptx2);

  auto vbkcontaining = popminer->mineVbkBlocks(*tipB, 1);
  EXPECT_EQ(vbkcontaining->height, 39);
  tipB = vbkcontaining;
  auto vtbcontaining = *vbkcontaining->header;

  // since chain B contains an endorsement of KS period 20-40, now it has to
  // be active
  EXPECT_EQ(*popminer->vbk().getBestChain().tip(), *tipB);

  // endorse block 26 in chain B, containing is B40
  auto missingVbkBlock = popminer->mineVbkBlocks(*tipB, 1);
  tipB = missingVbkBlock;
  auto vbkpoptx = popminer->endorseVbkBlock(
      *tipB->getAncestor(26)->header, getLastKnownBtcBlock(), state);
  popminer->vbkmempool.push_back(vbkpoptx);
  tipB = popminer->mineVbkBlocks(*tipB, 1);
  ASSERT_EQ(tipB->height, 41);

  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  // mine 10 alt blocks
  mineAltBlocks(10, chain);

  // endorse ALT5
  auto vbktx1 =
      popminer->createVbkTxEndorsingAltBlock(generatePublicationData(chain[5]));
  auto atv1 = popminer->generateATV(vbktx1, getLastKnownVbkBlock(), state);
  ASSERT_EQ(atv1.containingBlock.height, 42);

  // mine 10 more blocks on top of tipB
  tipB = popminer->mineVbkBlocks(*tipB, 10);

  //! act: add ATV1, VTB1 to ALT9. should be valid.
  auto vtb1 = popminer->vbkPayloads[vtbcontaining.getHash()][0];
  PopData p1;
  p1.atvs = {atv1};
  p1.vtbs = {vtb1};
  fillVbkContext(p1.context,
                 getLastKnownVbkBlock(),
                 atv1.containingBlock.getHash(),
                 popminer->vbk());

  ASSERT_TRUE(alttree.addPayloads(chain[9], p1, state));
  ASSERT_TRUE(alttree.setState(chain[9].hash, state)) << state.toString();

  //! act: add ATV2, VTB2 to ALT10
  // endorse ALT5
  auto vbktx2 =
      popminer->createVbkTxEndorsingAltBlock(generatePublicationData(chain[5]));
  auto atv2 = popminer->generateATV(vbktx1, getLastKnownVbkBlock(), state);
  ASSERT_EQ(atv1.containingBlock.height, 42);

  auto vtb2 = popminer->vbkPayloads[vtbcontaining.getHash()][1];
  PopData p2;
  p2.atvs = {atv2};
  // break VTB2: break hash of containing block
  vtb2.containingBlock.previousBlock = uint96::fromHex("abcdef");
  p2.vtbs = {vtb2};

  ASSERT_TRUE(alttree.addPayloads(chain[10], p2, state));
  ASSERT_FALSE(alttree.setState(chain[10].hash, state));
}

TEST_F(PopFrInvalidVbkChainTest, DuplicateEndorsementsInForks) {
  popminer->mineBtcBlocks(97);

  auto *vbkForkPoint = popminer->mineVbkBlocks(20);

  auto *tipA = popminer->mineVbkBlocks(*vbkForkPoint, 19);
  auto *tipB = popminer->mineVbkBlocks(*vbkForkPoint, 19);

  // make sure we have actually forked the blockchain
  ASSERT_NE(tipA->header, tipB->header);

  // 98 = contains endorsement of 20, present in A40 and B40
  ASSERT_EQ(97, popminer->btc().getBestChain().tip()->height);
  ASSERT_EQ(39, tipA->height);
  ASSERT_EQ(39, tipB->height);
  auto endorsedBlock = vbkForkPoint;
  ASSERT_EQ(20, endorsedBlock->height);

  auto btcTx = popminer->createBtcTxEndorsingVbkBlock(*endorsedBlock->header);
  auto *btcTip = popminer->mineBtcBlocks(1);

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcTip->header,
      btcTx,
      *endorsedBlock->header,
      popminer->getBtcParams().getGenesisBlock().getHash());
  popminer->mineVbkBlocks(*tipA, 1);

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcTip->header,
      btcTx,
      *endorsedBlock->header,
      popminer->getBtcParams().getGenesisBlock().getHash());
  EXPECT_THROW(popminer->mineVbkBlocks(*tipB, 1), std::domain_error);
}
