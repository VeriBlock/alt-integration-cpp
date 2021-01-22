// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * BTC:
 * o-o-o-98-[...]-100-[...]-102-103-[...]-110-111-112-113 (tip, longest)
 *
 * 98 = contains endorsement of 20, present in A40
 * 99 = contains endorsement of 20, present in B40
 * 100 = contains endorsement of A40 present in A50
 * 101 = contains endorsement of B40 present in B50
 * 102 = contains endorsement of A60, present in A70
 * 103 = contains endorsement of B60, present in B61
 * 111 = contains endorsement of B80, present in B85
 * 114 = contains endorsement of A80, present in A92
 *
 * VBK:
 *           /-A40-[...]-A60-[...]-A80-[...]-A100
 * o-o-o-o-20-o-B40-[...]-B60-[...]-B80-[...]-B95 (tip)
 *
 * B has better PoP score. Even though A started ahead with publication
 * of A60 to earlier BTC block, B80 had better PoP publication which
 * made its total score better.
 *
 * ALT: (BTC block tree)
 * o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o 100 blocks, no ATVs or VTBs
 * alt tree has VBK+BTC bootstrapped at the genesis blocks
 *
 */

struct Scenario5 : public ::testing::Test, public PopTestFixture {
  uint256 vbkContextStart;

  Scenario5()
      : vbkContextStart(GetRegTestBtcBlock().getHash()) {}

  void mineVbkFork(BlockIndex<VbkBlock>*& tip, size_t blockCount) {
    tip = popminer->mineVbkBlocks(blockCount, *tip);
  }

  void popMineVbkFork(BlockIndex<VbkBlock>*& tip, int endorsedAncestorHeight) {
    auto endorsedBlock = tip->getAncestor(endorsedAncestorHeight)->getHeader();
    auto btcTx = popminer->createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcTip = popminer->mineBtcBlocks(1);

    popminer->createVbkPopTxEndorsingVbkBlock(
        btcTip->getHeader(), btcTx, endorsedBlock, vbkContextStart);
    mineVbkFork(tip, 1);
  }

  void assertBestChain(BlockIndex<VbkBlock>* tipA, BlockIndex<VbkBlock>* tipB) {
    ASSERT_TRUE(popminer->vbk().getBestChain().contains(tipA));
    ASSERT_FALSE(popminer->vbk().getBestChain().contains(tipB));
  }
};

TEST_F(Scenario5, scenario_5) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  popminer->mineBtcBlocks(97);

  auto* vbkForkPoint = popminer->mineVbkBlocks(20);

  auto* tipA = popminer->mineVbkBlocks(19, *vbkForkPoint);
  auto* tipB = popminer->mineVbkBlocks(19, *vbkForkPoint);

  // make sure we have actually forked the blockchain
  ASSERT_NE(tipA->getHeader(), tipB->getHeader());

  {  // 98 = contains endorsement of 20, present in A40 and B40
    ASSERT_EQ(97, popminer->btc().getBestChain().tip()->getHeight());
    ASSERT_EQ(39, tipA->getHeight());
    ASSERT_EQ(39, tipB->getHeight());
    auto endorsedBlock = vbkForkPoint;
    ASSERT_EQ(20, endorsedBlock->getHeight());

    auto btcTx =
        popminer->createBtcTxEndorsingVbkBlock(endorsedBlock->getHeader());
    auto* btcTip = popminer->mineBtcBlocks(1);

    popminer->createVbkPopTxEndorsingVbkBlock(btcTip->getHeader(),
                                              btcTx,
                                              endorsedBlock->getHeader(),
                                              vbkContextStart);
    mineVbkFork(tipA, 1);

    btcTx = popminer->createBtcTxEndorsingVbkBlock(endorsedBlock->getHeader());
    btcTip = popminer->mineBtcBlocks(1);
    popminer->createVbkPopTxEndorsingVbkBlock(btcTip->getHeader(),
                                              btcTx,
                                              endorsedBlock->getHeader(),
                                              vbkContextStart);
    mineVbkFork(tipB, 1);
  }

  mineVbkFork(tipA, 9);
  mineVbkFork(tipB, 9);

  {
    //100 = contains endorsement of A40 present in A50
    ASSERT_EQ(99, popminer->btc().getBestChain().tip()->getHeight());
    ASSERT_EQ(49, tipA->getHeight());
    auto& endorsedBlockA = tipA->getAncestor(40)->getHeader();
    ASSERT_EQ(49, tipB->getHeight());
    auto& endorsedBlockB = tipB->getAncestor(40)->getHeader();

    auto btcTxA = popminer->createBtcTxEndorsingVbkBlock(endorsedBlockA);
    auto* btcTip = popminer->mineBtcBlocks(1);

    popminer->createVbkPopTxEndorsingVbkBlock(
        btcTip->getHeader(), btcTxA, endorsedBlockA, vbkContextStart);
    mineVbkFork(tipA, 1);

    // 101 = contains endorsement of B40 present in B50
    auto btcTxB = popminer->createBtcTxEndorsingVbkBlock(endorsedBlockB);
    btcTip = popminer->mineBtcBlocks(1);

    popminer->createVbkPopTxEndorsingVbkBlock(
        btcTip->getHeader(), btcTxB, endorsedBlockB, vbkContextStart);
    mineVbkFork(tipB, 1);
  }

  mineVbkFork(tipA, 19);

  // 102 = contains endorsement of A60, present in A70
  ASSERT_EQ(101, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(69, tipA->getHeight());
  popMineVbkFork(tipA, 60);

  assertBestChain(tipA, tipB);  // A is the best chain as it has an endorsement

  mineVbkFork(tipB, 10);

  // 103 = contains endorsement of B60, present in B61
  ASSERT_EQ(102, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(60, tipB->getHeight());
  popMineVbkFork(tipB, 60);

  assertBestChain(tipA, tipB);  // A is still the best chain as it has an
                                // earlier endorsement of block 60

  popminer->mineBtcBlocks(7);
  mineVbkFork(tipB, 23);

  // 111 = contains endorsement of B80, present in B85
  ASSERT_EQ(110, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(84, tipB->getHeight());
  popMineVbkFork(tipB, 80);

  assertBestChain(
      tipB, tipA);  // B becomes the best chain as it has more endorsements

  popminer->mineBtcBlocks(2);
  mineVbkFork(tipA, 21);

  // 114 = contains endorsement of A80, present in A92
  ASSERT_EQ(113, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(91, tipA->getHeight());
  popMineVbkFork(tipA, 80);

  assertBestChain(
      tipB,
      tipA);  // B remains the best chain as A has a late block 80 endorsement

  mineVbkFork(tipA, 8);
  mineVbkFork(tipB, 10);

  // check the final outcome
  ASSERT_EQ(114, popminer->btc().getBestChain().tip()->getHeight());
  ASSERT_EQ(100, tipA->getHeight());
  ASSERT_EQ(95, tipB->getHeight());

  // B has a better PoP score. Even though A started ahead with publication
  // of A60 to earlier BTC block, B80 had better PoP publication which
  // made its total score better.
  assertBestChain(tipB, tipA);
}
