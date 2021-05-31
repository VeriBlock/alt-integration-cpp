// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct VbkBlockFinalization : public ::testing::Test, public PopTestFixture {
  VbkBlockTree* tree{nullptr};
  size_t vbkTotalBlocks = 0;

  void SetUp() override {
    tree = &popminer->vbk();
    vbkTotalBlocks = tree->getBlocks().size();
  }
};

TEST_F(VbkBlockFinalization, BasicTest) {
  auto* finalizedBlock = popminer->mineVbkBlocks(100);
  auto* vbkendorsed = popminer->mineVbkBlocks(7);
  ASSERT_LT(popminer->vbk().getBestChain().chainHeight(),
            popminer->vbk().getParams().getOldBlocksWindow());
  popminer->mineBtcBlocks(100);
  ASSERT_LT(popminer->btc().getBestChain().chainHeight(),
            popminer->btc().getParams().getOldBlocksWindow());
  auto btctx0 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());
  auto* btctip = popminer->mineBtcBlocks(1, {btctx0});
  auto poptx0 = popminer->createVbkPopTxEndorsingVbkBlock(
      btctip->getHeader(),
      btctx0,
      vbkendorsed->getHeader(),
      // equals to genesis
      tree->btc().getBestChain().tip()->getHash());
  auto* vbktip = popminer->mineVbkBlocks(1, *vbkendorsed, {poptx0});
  ASSERT_TRUE(tree->setState(vbktip->getHash(), state));

  ASSERT_EQ(btctip->getHash(),
            tree->btc().getBestChain().tip()->getHash());

  size_t btcTotalBlocks = tree->btc().getBlocks().size();
  vbkTotalBlocks = tree->getBlocks().size();

  ASSERT_TRUE(tree->setState(vbktip->pprev->getHash(), state));

  ASSERT_TRUE(tree->finalizeBlock(*finalizedBlock, state));
  ASSERT_TRUE(tree->setState(vbktip->getHash(), state));
  ASSERT_EQ(tree->getBlocks().size(), vbkTotalBlocks);
  ASSERT_EQ(tree->btc().getBlocks().size(), btcTotalBlocks);
}

TEST_F(VbkBlockFinalization, OverBtcLimitTest) {
  auto* finalizedBlock = popminer->mineVbkBlocks(100);

  auto* vbkendorsed = popminer->mineVbkBlocks(7);
  popminer->mineBtcBlocks(tree->btc().getParams().getOldBlocksWindow() * 2);
  auto btctx0 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());
  auto* btctip = popminer->mineBtcBlocks(1, {btctx0});
  auto poptx0 = popminer->createVbkPopTxEndorsingVbkBlock(
      btctip->getHeader(),
      btctx0,
      vbkendorsed->getHeader(),
      // equals to genesis
      tree->btc().getBestChain().tip()->getHash());
  auto* vbktip = popminer->mineVbkBlocks(1, *vbkendorsed, {poptx0});
  ASSERT_TRUE(tree->setState(vbktip->getHash(), state));

  ASSERT_EQ(btctip->getHash(), tree->btc().getBestChain().tip()->getHash());

  ASSERT_TRUE(tree->setState(vbktip->pprev->getHash(), state));

  ASSERT_TRUE(tree->finalizeBlock(*finalizedBlock, state));
  ASSERT_DEATH(tree->setState(vbktip->getHash(), state), "");
}