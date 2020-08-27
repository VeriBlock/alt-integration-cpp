// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"

using namespace altintegration;

struct MemPoolPrioritizationFixture : public ::testing::Test,
                                      public PopTestFixture {};

// Compare the same vtbs
TEST_F(MemPoolPrioritizationFixture, vtb_isStronglyEquivalent_scenario1_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  EXPECT_TRUE(popminer->vbk().areStronglyEquivalent(vtbs[0], vtbs[0]));
  EXPECT_EQ(popminer->vbk().weaklyCompare(vtbs[0], vtbs[0]), 0);
}

// Compare vtbs that are contains in the same chain with the same vbkPop
// transactions
TEST_F(MemPoolPrioritizationFixture, vtb_isStronglyEquivalent_scenario2_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb1 = vtbs[0];

  ASSERT_TRUE(popminer->vbkmempool.empty());
  popminer->vbkmempool.push_back(vbkPopTx);
  ASSERT_FALSE(popminer->vbkmempool.empty());
  vbkTip = popminer->mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_TRUE(popminer->vbk().areStronglyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(popminer->vbk().areStronglyEquivalent(vtb2, vtb1));
  EXPECT_EQ(popminer->vbk().weaklyCompare(vtb1, vtb2), 0);
}

// Compare vtbs that are contains in the different chains with the same vbkPop
// transactions
TEST_F(MemPoolPrioritizationFixture, vtb_isStronglyEquivalent_scenario3_test) {
  auto* vbkFork = popminer->mineVbkBlocks(50);
  auto* vbkTip = popminer->mineVbkBlocks(2);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb1 = vtbs[0];

  ASSERT_TRUE(popminer->vbkmempool.empty());
  popminer->vbkmempool.push_back(vbkPopTx);
  ASSERT_FALSE(popminer->vbkmempool.empty());
  auto* vbkForkTip = popminer->mineVbkBlocks(*vbkFork, 1);
  ASSERT_EQ(vbkForkTip->getHeight(), vbkFork->getHeight() + 1);
  ASSERT_NE(vbkForkTip->getHash(),
            popminer->vbk().getBestChain().tip()->getHash());
  vtbs = popminer->vbkPayloads[vbkForkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_TRUE(popminer->vbk().areStronglyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(popminer->vbk().areStronglyEquivalent(vtb2, vtb1));
  EXPECT_EQ(popminer->vbk().weaklyCompare(vtb1, vtb2), 0);
}

// Compare vtbs that are not equal
TEST_F(MemPoolPrioritizationFixture, vtb_isStronglyEquivalent_scenario4_test) {
  // mine 65 vbk blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb2, vtb1));

  EXPECT_TRUE(popminer->vbk().areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(popminer->vbk().areWeaklyEquivalent(vtb2, vtb1));
}

TEST_F(MemPoolPrioritizationFixture, vtb_isWeaklyEquivalent_scenario1_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  EXPECT_TRUE(popminer->vbk().areWeaklyEquivalent(vtbs[0], vtbs[0]));
}

TEST_F(MemPoolPrioritizationFixture, vtb_isWeaklyEquivalent_scenario2_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  popminer->mineBtcBlocks(1);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb2, vtb1));

  EXPECT_TRUE(popminer->vbk().areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(popminer->vbk().areWeaklyEquivalent(vtb2, vtb1));
}

TEST_F(MemPoolPrioritizationFixture, vtb_isWeaklyEquivalent_scenario3_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 Btc blocks
  auto* btcFork = popminer->mineBtcBlocks(10);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  // mine btc fork
  popminer->mineBtcBlocks(*btcFork, 10);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(popminer->vbk().areStronglyEquivalent(vtb2, vtb1));

  EXPECT_FALSE(popminer->vbk().areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(popminer->vbk().areWeaklyEquivalent(vtb2, vtb1));

  ASSERT_DEATH(popminer->vbk().weaklyCompare(vtb1, vtb2),
               "vtbs should be weakly equivalent");
}