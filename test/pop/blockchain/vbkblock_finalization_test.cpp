// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct VbkBlockFinalization : public ::testing::Test, public PopTestFixture {
  VbkBlockTree* tree{nullptr};

  BlockIndex<VbkBlock>* A100{nullptr};

  size_t vbkTotalBlocks = 0;

  void SetUp() override {
    tree = &popminer->vbk();

    A100 = popminer->mineVbkBlocks(100);

    vbkTotalBlocks = tree->getBlocks().size();
    ASSERT_EQ(vbkTotalBlocks, A100->getHeight() + 1);
  }
};

TEST_F(VbkBlockFinalization, BasicTest) {
  auto vbkendorsed = popminer->mineVbkBlocks(7);
  auto btctx0 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());
  auto btcBlockOfProof0 = popminer->mineBtcBlocks(1, {btctx0});
  auto btcTip =
      popminer->mineBtcBlocks(tree->btc().getParams().getOldBlocksWindow() * 2);
  auto poptx0 = popminer->createVbkPopTxEndorsingVbkBlock(
      btcBlockOfProof0->getHeader(),
      btctx0,
      vbkendorsed->getHeader(),
      // equals to genesis
      tree->btc().getBestChain().tip()->getHash());
  auto vtb0containing = popminer->mineVbkBlocks(1, *vbkendorsed, {poptx0});
  auto VTB0 = popminer->createVTB(vtb0containing->getHeader(), poptx0);

  ASSERT_TRUE(tree->addPayloads(vtb0containing->getHash(), {VTB0}, state));
  ASSERT_TRUE(tree->setState(vtb0containing->getHash(), state));
  ASSERT_EQ(btcTip->getHash(), tree->btc().getBestChain().tip()->getHash());

  size_t btcTotalBlocks = tree->btc().getBlocks().size();

  ASSERT_TRUE(tree->setState(vtb0containing->pprev->getHash(), state));

  ASSERT_TRUE(tree->finalizeBlock(A100->getHash()));
  ASSERT_TRUE(tree->setState(vtb0containing->getHash(), state));
  ASSERT_LT(tree->getBlocks().size(), vbkTotalBlocks);
  ASSERT_LT(tree->btc().getBlocks().size(), btcTotalBlocks);
}