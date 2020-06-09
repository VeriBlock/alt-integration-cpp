// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <exception>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"

using namespace altintegration;

struct PopVbkForkResolution : public ::testing::Test, public PopTestFixture {};

// TODO: temporary disable
// TEST_F(PopVbkForkResolution, TooLateToAddPayloads) {
//  popminer->mineVbkBlocks(2);
//  auto vbkpoptx =
//      popminer->endorseVbkBlock(*popminer->vbk().getBestChain()[1]->header,
//                                getLastKnownBtcBlock(),
//                                state);
//  popminer->vbkmempool.push_back(vbkpoptx);
//
//  // try to add payloads to block pre latest allowed to overwrite
//  auto limit = popminer->getVbkParams().getHistoryOverwriteLimit();
//  auto vbkcontaining = popminer->mineVbkBlocks(1);
//  popminer->mineVbkBlocks(limit);
//
//  auto& vtb = popminer->vbkPayloads.at(vbkcontaining->getHash()).at(0);
//
//  ASSERT_FALSE(
//      popminer->vbk().addPayloads(vbkcontaining->getHash(), {vtb}, state));
//  ASSERT_EQ(state.GetPath(), "VBK-too-late");
//}

TEST_F(PopVbkForkResolution, A_1_endorsement_B_longer) {
  // start with 10 BTC blocks
  popminer->mineBtcBlocks(10);
  // start with 65 VBK blocks
  auto* chainBtip = popminer->mineVbkBlocks(65);

  // current best chain is at block 65, chain B
  ASSERT_EQ(popminer->vbk().getBestChain().tip(), chainBtip);

  auto* forkPoint = popminer->vbk().getBestChain().tip()->getAncestor(50);
  auto* chainAtip = popminer->mineVbkBlocks(*forkPoint, 10);

  // best chain is still B
  ASSERT_EQ(popminer->vbk().getBestChain().tip(), chainBtip);

  // create 1 endorsement and put it into
  auto Atx1 = popminer->createBtcTxEndorsingVbkBlock(*chainAtip->header);
  auto Abtccontaining1 = popminer->mineBtcBlocks(1);
  ASSERT_TRUE(popminer->btc().getBestChain().contains(Abtccontaining1));

  auto Apoptx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      *Abtccontaining1->header,
      Atx1,
      *chainAtip->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  // state is still at chain B
  ASSERT_EQ(popminer->vbk().getBestChain().tip(), chainBtip);

  // mine one block on top of smaller chain A.
  // this block will contain endorsement of chain A
  auto Avbkcontaining1 = popminer->mineVbkBlocks(*chainAtip, 1);

  // chain changed to chain A, because its POP score is higher
  ASSERT_EQ(*popminer->vbk().getBestChain().tip(), *Avbkcontaining1);

  // and now endorse block 60 of chain B
  // mine 5 BtcBlocks
  popminer->mineBtcBlocks(5);

  auto* B60 = chainBtip->getAncestor(60);
  auto Btx1 = popminer->createBtcTxEndorsingVbkBlock(*B60->header);
  auto Bbtccontaining1 = popminer->mineBtcBlocks(1);
  ASSERT_TRUE(popminer->btc().getBestChain().contains(Bbtccontaining1));

  auto Bpoptx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      *Bbtccontaining1->header,
      Btx1,
      *B60->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  popminer->mineVbkBlocks(*chainBtip, 1);

  // chain is still at chain A, because endorsement was erlier
  EXPECT_EQ(*popminer->vbk().getBestChain().tip(), *Avbkcontaining1);
}

TEST_F(PopVbkForkResolution, endorsement_not_in_the_BTC_main_chain) {
  // We test that we have an endorsement in the Btc block that is not in the
  // main chain. This test will validate this scenario in the
  // getProtoKeystoneContext() function

  // start with 30 BTC blocks
  auto* btcBlockTip1 = popminer->mineBtcBlocks(30);
  // start with 65 VBK blocks
  auto* vbkBlockTip = popminer->mineVbkBlocks(65);

  ASSERT_EQ(btcBlockTip1->getHash(),
            popminer->btc().getBestChain().tip()->getHash());

  auto* btcForkPoint = btcBlockTip1->getAncestor(15);

  // create 1 endorsement and put it into
  auto Atx1 = popminer->createBtcTxEndorsingVbkBlock(*vbkBlockTip->header);

  auto* btcBlockTip2 = popminer->mineBtcBlocks(*btcForkPoint, 1);

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcBlockTip2->header,
      Atx1,
      *vbkBlockTip->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  // Test the same case but in the getProtoKeystoneContext() function
  // make btcBlockTtip2 active chain tip
  btcBlockTip2 = popminer->mineBtcBlocks(*btcBlockTip2, 40);
  ASSERT_EQ(popminer->btc().getBestChain().tip()->getHash(),
            btcBlockTip2->getHash());

  vbkBlockTip = popminer->mineVbkBlocks(1);

  EXPECT_EQ(vbkBlockTip->pprev->endorsedBy.size(), 1);

  Chain<BlockIndex<VbkBlock>> chain(0, vbkBlockTip);

  auto context = internal::getProtoKeystoneContext(
      chain, popminer->btc(), popminer->getVbkParams());

  EXPECT_EQ(context[context.size() - 1].referencedByBlocks.size(), 1);

  // change active chain to the another branch
  btcBlockTip1 = popminer->mineBtcBlocks(*btcBlockTip1, 90);
  ASSERT_EQ(*popminer->btc().getBestChain().tip(), *btcBlockTip1);

  context = internal::getProtoKeystoneContext(
      chain, popminer->btc(), popminer->getVbkParams());

  EXPECT_EQ(context[context.size() - 1].referencedByBlocks.size(), 0);
}

TEST_F(PopVbkForkResolution, endorsement_not_in_the_Vbk_chain) {
  // We test that we have an endorsement that endorse VeriBlock block from the
  // another chain
  //        / [] - [block_n] - []
  // ... - []
  //        \ [] - [] - [this block contains endorsement of the 'block_n']

  // start with 30 BTC blocks
  auto* btcBlockTip1 = popminer->mineBtcBlocks(30);

  ASSERT_EQ(btcBlockTip1->getHash(),
            popminer->btc().getBestChain().tip()->getHash());

  // start with 65 VBK blocks
  auto* vbkForkpoint = popminer->mineVbkBlocks(65);

  auto* vbkBlockTip1 = popminer->mineVbkBlocks(40);

  // make a VbkFork
  auto* vbkBlockTip2 = popminer->mineVbkBlocks(*vbkForkpoint, 20);

  ASSERT_EQ(popminer->vbk().getBestChain().tip()->getHash(),
            vbkBlockTip1->getHash());

  ASSERT_FALSE(popminer->vbk().getBestChain().contains(vbkBlockTip2));

  auto* endorsedVbkBlock = vbkBlockTip1->getAncestor(vbkBlockTip1->height - 10);

  Chain<BlockIndex<VbkBlock>> forkChain(0, vbkBlockTip2);
  ASSERT_FALSE(forkChain.contains(endorsedVbkBlock));

  // create 1 endorsement and put it into
  auto Atx1 = popminer->createBtcTxEndorsingVbkBlock(*endorsedVbkBlock->header);

  btcBlockTip1 = popminer->mineBtcBlocks(1);
  EXPECT_EQ(*btcBlockTip1, *popminer->btc().getBestChain().tip());

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcBlockTip1->header,
      Atx1,
      *endorsedVbkBlock->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  auto vbktip1 = popminer->vbk().getBestChain().tip();
  // should not throw, as we removed call to 'invalidateSubtree'
  ASSERT_THROW(popminer->mineVbkBlocks(*vbkBlockTip2, 1), std::domain_error);
  auto vbktip3 = popminer->vbk().getBestChain().tip();

  ASSERT_EQ(*vbktip1, *vbktip3) << "tip has been changed wrongly";
}

TEST_F(PopVbkForkResolution, duplicate_endorsement_in_the_same_chain) {
  // We test that we have a duplicate endorsement in the same VeriBlock chain
  // ... - [] - []- [endorsement_1] - []- [endorsement_1 (the same
  //  endorsement)]

  // start with 30 BTC blocks
  auto* btcBlockTip1 = popminer->mineBtcBlocks(30);

  ASSERT_EQ(btcBlockTip1->getHash(),
            popminer->btc().getBestChain().tip()->getHash());

  // start with 65 VBK blocks
  auto* vbkBlockTip = popminer->mineVbkBlocks(65);

  ASSERT_EQ(popminer->vbk().getBestChain().tip()->getHash(),
            vbkBlockTip->getHash());

  auto* endorsedVbkBlock = vbkBlockTip->getAncestor(vbkBlockTip->height - 10);

  // create 1 endorsement and put it into
  auto Atx1 = popminer->createBtcTxEndorsingVbkBlock(*endorsedVbkBlock->header);

  btcBlockTip1 = popminer->mineBtcBlocks(1);
  ASSERT_EQ(btcBlockTip1->getHash(),
            popminer->btc().getBestChain().tip()->getHash());

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcBlockTip1->header,
      Atx1,
      *endorsedVbkBlock->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  // mine the first endorsement
  popminer->mineVbkBlocks(1);
  ASSERT_EQ(endorsedVbkBlock->endorsedBy.size(), 1);

  popminer->createVbkPopTxEndorsingVbkBlock(
      *btcBlockTip1->header,
      Atx1,
      *endorsedVbkBlock->header,
      popminer->getBtcParams().getGenesisBlock().getHash());

  // mine the second the same endorsement
  auto vbktip1 = popminer->vbk().getBestChain().tip();
  ASSERT_THROW(popminer->mineVbkBlocks(1), std::domain_error);
  auto vbktip3 = popminer->vbk().getBestChain().tip();

  ASSERT_EQ(*vbktip1, *vbktip3);
}
