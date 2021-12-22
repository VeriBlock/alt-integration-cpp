// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

struct AddPayloadsChained : public ::testing::Test, public PopTestFixture {};

TEST_F(AddPayloadsChained, addPayloadsChained) {
  PopData popData;

  std::vector<VbkPopTx> vbkPopTxs;

  auto vbkPopTx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      popminer->vbkTip()->getHeader(), popminer->btcTip()->getHash());
  auto vbkPopTx2 = popminer->createVbkPopTxEndorsingVbkBlock(
      popminer->vbkTip()->getHeader(), popminer->btcTip()->getHash());

  // both VTBs should be contained in the same block
  BlockIndex<VbkBlock>* containingVbkBlock =
      popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(containingVbkBlock->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(containingVbkBlock->getHeader(), vbkPopTx2);
  popData.vtbs = {vtb1, vtb2};
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock->getHash(),
                 popminer->vbk());

  payloadsProvider.writePayloads(popData);

  // BTC contexts should be empty
  ASSERT_EQ(popData.vtbs[0].transaction.blockOfProofContext.size(), 0);
  ASSERT_EQ(popData.vtbs[1].transaction.blockOfProofContext.size(), 0);

  // VTB blocks of proof and the tip of alttree.btc() (== genesis block) form a
  // chain
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHash(),
            popData.vtbs[0].transaction.blockOfProof.getPreviousBlock());
  ASSERT_EQ(popData.vtbs[0].transaction.blockOfProof.getHash(),
            popData.vtbs[1].transaction.blockOfProof.getPreviousBlock());

  // manually add the VBK context to alttree
  for (auto& block : popData.context) {
    ASSERT_TRUE(alttree.vbk().acceptBlockHeader(block, state)) << state.toString();
  }

  auto* containingIndex =
      alttree.vbk().getBlockIndex(containingVbkBlock->getHash());
  ASSERT_NE(containingIndex, nullptr)
      << "the containing VBK block should exist";

  ASSERT_TRUE(alttree.vbk().addPayloads(
      containingVbkBlock->getHash(), popData.vtbs, state))
      << state.toString();
}
