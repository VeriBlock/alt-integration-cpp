// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * Setup following tree:
 *                        /-E503
 *             / -B501-B502-B503
 * 0.....250..500-A501-A502-A503-A504 (tip)
 *        |    \ -C501-C502
 *        |          \-D502
 *        \-Z251
 *
 */
struct AltBlockFinalization : public ::testing::Test, public PopTestFixture {
  BlockIndex<AltBlock> *A504 = nullptr;
  BlockIndex<AltBlock> *B503 = nullptr;
  BlockIndex<AltBlock> *E503 = nullptr;
  BlockIndex<AltBlock> *C502 = nullptr;
  BlockIndex<AltBlock> *D502 = nullptr;
  BlockIndex<AltBlock> *Z251 = nullptr;

  size_t totalBlocks = 0;

  void SetUp() override {
    altparam.mMaxReorgDistance = 500;
    altparam.mEndorsementSettlementInterval = 50;
    altparam.mPreserveBlocksBehindFinal = 50;

    A504 = mineAltBlocks(alttree.getRoot(), 504);
    B503 = mineAltBlocks(*A504->getAncestor(500), 3);
    E503 = mineAltBlocks(*B503->getAncestor(502), 1);
    C502 = mineAltBlocks(*A504->getAncestor(500), 2);
    D502 = mineAltBlocks(*C502->getAncestor(501), 1);
    Z251 = mineAltBlocks(*A504->getAncestor(250), 1);

    ASSERT_TRUE(alttree.setState(*A504, state)) << state.toString();

    totalBlocks = alttree.getBlocks().size();
  }
};

TEST_F(AltBlockFinalization, FinalizeRoot) {
  auto *bootstrap = alttree.getBestChain().first();
  ASSERT_TRUE(alttree.finalizeBlock(*bootstrap, state));
  // unchanged
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);
  assertTreeTips(alttree, {A504, B503, C502, D502, E503, Z251});
  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization, FinalizeTip0Window) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  auto *tip = alttree.getBestChain().tip();
  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A251, which has one parallel block Z251 (tip).
TEST_F(AltBlockFinalization, FinalizeA251) {
  auto *A251 = A504->getAncestor(251);
  ASSERT_TRUE(alttree.finalizeBlock(*A251, state));
  ASSERT_TRUE(alttree.setState(A251->getHash(), state)) << state.toString();

  auto *A201 = A251->getAncestor(201);
  ASSERT_TRUE(A201);
  ASSERT_FALSE(A201->pprev);
  // total - 201 - 1 (Z251 tip)
  ASSERT_EQ(alttree.getBlocks().size(), 311);
  EXPECT_EQ(alttree.getRoot().getHash(), A201->getHash());

  assertTreeTips(alttree, {A504, B503, C502, D502, E503});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A501. all tips starting at A500 should be removed. single
// chain should remain.
TEST_F(AltBlockFinalization, FinalizeA501) {
  auto *A501 = A504->getAncestor(501);
  ASSERT_TRUE(alttree.finalizeBlock(*A501, state));
  ASSERT_TRUE(alttree.setState(A501->getHash(), state)) << state.toString();

  // 501, 502, 503, 504 + 50 prev blocks
  EXPECT_EQ(alttree.getBlocks().size(), 4 + 50);

  // presumably first block in mem
  auto *A451 = A501->getAncestor(451);
  ASSERT_TRUE(A451);
  ASSERT_FALSE(A451->pprev);
  EXPECT_EQ(alttree.getRoot().getHash(), A451->getHash());
  assertTreeTips(alttree, {A504});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A500. all tips that start at 500 should remain.
TEST_F(AltBlockFinalization, FinalizeA500) {
  auto *A500 = A504->getAncestor(500);
  ASSERT_TRUE(alttree.finalizeBlock(*A500, state));
  ASSERT_TRUE(alttree.setState(A500->getHash(), state)) << state.toString();

  // 50 prev blocks + 5 chain A + 2 chain C + 1 chain D + 3 chain B + 1 chain E
  EXPECT_EQ(alttree.getBlocks().size(), 50 + 5 + 2 + 1 + 3 + 1);

  // presumably first block in mem
  auto *A450 = A500->getAncestor(450);
  ASSERT_TRUE(A450);
  ASSERT_FALSE(A450->pprev);
  EXPECT_EQ(alttree.getRoot().getHash(), A450->getHash());
  assertTreeTips(alttree, {A504, B503, C502, D502, E503});
  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization, FinalizeActiveChainOneByOne) {
  Chain<BlockIndex<AltBlock>> chain = alttree.getBestChain();
  for (auto *index : chain) {
    ASSERT_TRUE(alttree.finalizeBlock(*index, state)) << index->getHeight();
  }
  ASSERT_TRUE(alttree.setState(alttree.getBestChain().tip()->getHash(), state))
      << state.toString();

  assertTreeTips(alttree, {alttree.getBestChain().tip()});
  assertTreesHaveNoOrphans(alttree);
}

struct VbkBlockFinalization : public ::testing::Test, public PopTestFixture {
  BlockIndex<AltBlock> *tip = nullptr;
  size_t totalBlocks = 0;

  void SetUp() override {
    altparam.mMaxReorgDistance = 500;
    altparam.mEndorsementSettlementInterval = 50;
    altparam.mPreserveBlocksBehindFinal = 50;

    tip = mineAltBlocks(alttree.getRoot(), 100);
    ASSERT_TRUE(alttree.setState(*tip, state));
    totalBlocks = alttree.getBlocks().size();
  }
};

TEST_F(VbkBlockFinalization, FinalizeVbkTip) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mEndorsementSettlementInterval = 0;
  vbkparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mOldBlocksWindow = 0;

  PopData altPayloads;

  auto* vbktip = popminer->mineVbkBlocks(10);
  fillVbkContext(altPayloads.context,
                 GetRegTestVbkBlock().getHash(),
                 vbktip->getHash(),
                 popminer->vbk());

  AltBlock containingBlock = generateNextBlock(alttree.getBestChain().tip()->getHeader());
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state))
      << state.toString();
  ASSERT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  EXPECT_TRUE(state.IsValid());
  vbktip = alttree.vbk().getBestChain().tip();
  ASSERT_EQ(alttree.vbk().getBlocks().size(), 11);

  // mine ALT block without any contained VBK context
  tip = mineAltBlocks(*alttree.getBestChain().tip(), 1);
  ASSERT_EQ(alttree.getBlocks().size(), 103);

  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});

  ASSERT_EQ(alttree.vbk().getBlocks().size(), 1);
  assertTreeTips(alttree.vbk(), {vbktip});

  assertTreesHaveNoOrphans(alttree);
}