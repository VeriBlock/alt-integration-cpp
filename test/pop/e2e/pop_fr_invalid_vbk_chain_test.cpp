// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include "pop/util/pop_test_fixture.hpp"

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
  auto *tipB = popminer->mineVbkBlocks(28, *tipA->getAncestor(10));
  EXPECT_TRUE(cmp(*popminer->vbk().getBestChain().tip(), *tipA));

  // next block in B endorses block number 25 in its chain twice, and
  // containing is block 39
  auto vbkpoptx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      tipB->getAncestor(25)->getHeader(), getLastKnownBtcBlock());
  auto vbkpoptx2 = popminer->createVbkPopTxEndorsingVbkBlock(
      tipB->getAncestor(25)->getHeader(), getLastKnownBtcBlock());

  auto vbkcontaining =
      popminer->mineVbkBlocks(1, *tipB, {vbkpoptx1, vbkpoptx2});
  EXPECT_EQ(vbkcontaining->getHeight(), 39);
  tipB = vbkcontaining;
  auto vtbcontaining = vbkcontaining->getHeader();

  // since chain B contains an endorsement of KS period 20-40, now it has to
  // be active
  EXPECT_TRUE(cmp(*popminer->vbk().getBestChain().tip(), *tipB));

  // endorse block 26 in chain B, containing is B40
  auto missingVbkBlock = popminer->mineVbkBlocks(1, *tipB);
  tipB = missingVbkBlock;
  auto vbkpoptx = popminer->createVbkPopTxEndorsingVbkBlock(
      tipB->getAncestor(26)->getHeader(), getLastKnownBtcBlock());
  tipB = popminer->mineVbkBlocks(1, *tipB, {vbkpoptx});
  ASSERT_EQ(tipB->getHeight(), 41);

  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  // mine 10 alt blocks
  mineAltBlocks(8, chain, true, false);
  mineAltBlocks(2, chain, false, false);

  // endorse ALT5
  auto vbktx1 =
      popminer->createVbkTxEndorsingAltBlock(generatePublicationData(chain[5]));
  auto *vbkblock1 = popminer->mineVbkBlocks(1, {vbktx1});
  ATV atv1 = popminer->createATV(vbkblock1->getHeader(), vbktx1);
  ASSERT_EQ(atv1.blockOfProof.getHeight(), 42);

  // mine 10 more blocks on top of tipB
  tipB = popminer->mineVbkBlocks(10, *tipB);

  //! act: add ATV1, VTB1 to ALT9. should be valid.
  auto vtb1 = popminer->createVTB(vtbcontaining, vbkpoptx1);
  PopData p1;
  p1.atvs = {atv1};
  p1.vtbs = {vtb1};
  fillVbkContext(p1.context,
                 getLastKnownVbkBlock(),
                 atv1.blockOfProof.getHash(),
                 popminer->vbk());

  ASSERT_TRUE(AddPayloads(chain[9].getHash(), p1));
  ASSERT_TRUE(alttree.setState(chain[9].getHash(), state)) << state.toString();
  validateAlttreeIndexState(alttree, chain[9], p1);

  //! act: add ATV2, VTB2 to ALT10
  // endorse ALT5
  auto vbktx2 =
      popminer->createVbkTxEndorsingAltBlock(generatePublicationData(chain[5]));
  auto *vbkblock2 = popminer->mineVbkBlocks(1, {vbktx2});
  ATV atv2 = popminer->createATV(vbkblock2->getHeader(), vbktx2);
  ASSERT_EQ(atv1.blockOfProof.getHeight(), 42);

  auto vtb2 = popminer->createVTB(vtbcontaining, vbkpoptx2);
  PopData p2;
  p2.atvs = {atv2};
  // break VTB2: break hash of containing block
  vtb2.containingBlock.setPreviousBlock(uint96::fromHex("abcdef"));
  p2.vtbs = {vtb2};

  ASSERT_TRUE(AddPayloads(chain[10].getHash(), p2));
  ASSERT_FALSE(alttree.setState(chain[10].getHash(), state));
  // all payloads are marked valid as there's no correctly implemented
  // invalidation
  validateAlttreeIndexState(
      alttree, chain[10], p2, /*payloads_validation =*/true);
}

TEST_F(PopFrInvalidVbkChainTest, DuplicateEndorsementsInForks) {
  popminer->mineBtcBlocks(97);

  auto *vbkForkPoint = popminer->mineVbkBlocks(20);

  auto *tipA = popminer->mineVbkBlocks(19, *vbkForkPoint);
  auto *tipB = popminer->mineVbkBlocks(19, *vbkForkPoint);

  // make sure we have actually forked the blockchain
  ASSERT_NE(tipA->getHeader(), tipB->getHeader());

  // 98 = contains endorsement of 20, present in A40 and B40
  ASSERT_EQ(97, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(39, tipA->getHeight());
  ASSERT_EQ(39, tipB->getHeight());
  auto endorsedBlock = vbkForkPoint;
  ASSERT_EQ(20, endorsedBlock->getHeight());

  auto btcTx =
      popminer->createBtcTxEndorsingVbkBlock(endorsedBlock->getHeader());
  auto *btcTip = popminer->mineBtcBlocks(1, {btcTx});

  auto vbkPopTxA =
      popminer->createVbkPopTxEndorsingVbkBlock(btcTip->getHeader(),
                                                btcTx,
                                                endorsedBlock->getHeader(),
                                                GetRegTestBtcBlock().getHash());
  tipA = popminer->mineVbkBlocks(1, *tipA, {vbkPopTxA});

  auto vbkPopTxB =
      popminer->createVbkPopTxEndorsingVbkBlock(btcTip->getHeader(),
                                                btcTx,
                                                endorsedBlock->getHeader(),
                                                GetRegTestBtcBlock().getHash());
  tipB = popminer->mineVbkBlocks(1, *tipB, {vbkPopTxB});

  ASSERT_TRUE(tipA->isValid());
  ASSERT_TRUE(tipB->isValid());
}
