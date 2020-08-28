// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct AddPayloadsChained : public ::testing::Test, public PopTestFixture {};

TEST_F(AddPayloadsChained, addPayloadsChained) {
  PopData popData;

  std::vector<VbkPopTx> vbkPopTxs;

  vbkPopTxs.emplace_back(popminer->endorseVbkBlock(
      popminer->vbk().getBestChain().tip()->getHeader(),
      popminer->btc().getBestChain().tip()->getHash(),
      state));
  vbkPopTxs.emplace_back(popminer->endorseVbkBlock(
      popminer->vbk().getBestChain().tip()->getHeader(),
      popminer->btc().getBestChain().tip()->getHash(),
      state));

  // both VTBs should be contained in the same block
  VbkBlock containingVbkBlock =
      popminer->applyVTBs(popminer->vbk(), vbkPopTxs, state);

  popData.vtbs = popminer->vbkPayloads.at(containingVbkBlock.getHash());
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock.getHash(),
                 popminer->vbk());

  payloadsProvider.write(popData);

  // BTC contexts should be empty
  ASSERT_EQ(popData.vtbs[0].transaction.blockOfProofContext.size(), 0);
  ASSERT_EQ(popData.vtbs[1].transaction.blockOfProofContext.size(), 0);

  // VTB blocks of proof and the tip of alttree.btc() (== genesis block) form a
  // chain
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHash(),
            popData.vtbs[0].transaction.blockOfProof.previousBlock);
  ASSERT_EQ(popData.vtbs[0].transaction.blockOfProof.getHash(),
            popData.vtbs[1].transaction.blockOfProof.previousBlock);

  // manually add the VBK context to alttree
  for (auto& block : popData.context) {
    ASSERT_TRUE(alttree.vbk().acceptBlock(block, state)) << state.toString();
  }

  auto* containingIndex =
      alttree.vbk().getBlockIndex(containingVbkBlock.getHash());
  ASSERT_NE(containingIndex, nullptr)
      << "the containing VBK block should exist";

  ASSERT_TRUE(alttree.vbk().addPayloads(
      containingVbkBlock.getHash(), popData.vtbs, state))
      << state.toString();
}
