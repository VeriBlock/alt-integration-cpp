// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/mempool_fixture.hpp>
#include <pop/util/pop_test_fixture.hpp>

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
  size_t deallocated = 0;

  void SetUp() override {
    altparam.mMaxReorgBlocks = 500;
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

    alttree.onBlockBeforeDeallocated.connect(
        [&](const BlockIndex<AltBlock> &) { deallocated++; });
  }

  void assertAllPreviousBlocksFinalized(const BlockIndex<AltBlock> *block) {
    const auto *p = block;
    size_t iter = 0;
    while (p != nullptr) {
      ASSERT_TRUE(p->finalized)
          << "iteration: " << iter << ", block: " << p->toPrettyString();
      p = p->getPrev();
      iter++;
    }
  }
};

TEST_F(AltBlockFinalization, FinalizeRoot) {
  // save state
  save(alttree);

  auto *bootstrap = alttree.getBestChain().first();
  alttree.finalizeBlock(*bootstrap);

  // unchanged
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);
  assertTreeTips(alttree, {A504, B503, C502, D502, E503, Z251});
  assertTreesHaveNoOrphans(alttree);
  ASSERT_EQ(deallocated, 0);
  assertAllPreviousBlocksFinalized(bootstrap);
}

TEST_F(AltBlockFinalization, FinalizeTip0Window) {
  // save state
  save(alttree);

  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  auto *tip = alttree.getBestChain().tip();
  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, totalBlocks - 1);
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
  assertAllPreviousBlocksFinalized(tip);
}

TEST_F(AltBlockFinalization, FinalizeUnsavedBlocks) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;

  auto *tip = alttree.getBestChain().tip();
  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, 0);
  ASSERT_FALSE(tip->finalized);
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);

  // save state
  save(alttree);

  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, totalBlocks - 1);
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
  assertAllPreviousBlocksFinalized(tip);
}

TEST_F(AltBlockFinalization, FinalizeUnsavedBlocksForks) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;

  auto *tip = alttree.getBestChain().tip();
  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, 0);
  ASSERT_FALSE(tip->finalized);
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);

  // save state
  save(alttree);

  // mark all forks as dirty
  for (auto *currentTip : alttree.getTips()) {
    if (!alttree.getBestChain().contains(currentTip)) {
      currentTip->setDirty();
    }
  }

  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, 250);
  ASSERT_FALSE(tip->isDirty());
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  // do not finalize the whole tree
  ASSERT_LT(alttree.getBlocks().size(), totalBlocks);
  ASSERT_NE(alttree.getBlocks().size(), 1);

  // save state
  save(alttree);

  alttree.finalizeBlock(*tip);
  ASSERT_EQ(deallocated, totalBlocks - 1);
  ASSERT_TRUE(tip->finalized);
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
  assertAllPreviousBlocksFinalized(tip);
}

// finalize a block A251, which has one parallel block Z251 (tip).
TEST_F(AltBlockFinalization, FinalizeA251) {
  // save state
  save(alttree);

  auto *A251 = A504->getAncestor(251);
  alttree.finalizeBlock(*A251);
  assertAllPreviousBlocksFinalized(A251);

  ASSERT_TRUE(alttree.setState(A251->getHash(), state)) << state.toString();

  auto *A201 = A251->getAncestor(201);
  ASSERT_TRUE(A201);
  ASSERT_FALSE(A201->pprev);
  // total - 201 - 1 (Z251 tip)
  ASSERT_EQ(alttree.getBlocks().size(), 311);
  ASSERT_EQ(deallocated, 202);
  EXPECT_EQ(alttree.getRoot().getHash(), A201->getHash());

  assertTreeTips(alttree, {A504, B503, C502, D502, E503});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A501. all tips starting at A500 should be removed. single
// chain should remain.
TEST_F(AltBlockFinalization, FinalizeA501) {
  // save state
  save(alttree);

  auto *A501 = A504->getAncestor(501);
  alttree.finalizeBlock(*A501);
  ASSERT_TRUE(alttree.setState(A501->getHash(), state)) << state.toString();
  assertAllPreviousBlocksFinalized(A501);

  // 501, 502, 503, 504 + 50 prev blocks
  EXPECT_EQ(alttree.getBlocks().size(), 4 + 50);

  // presumably first block in mem
  auto *A451 = A501->getAncestor(451);
  ASSERT_TRUE(A451);
  ASSERT_FALSE(A451->pprev);
  ASSERT_EQ(deallocated, 459);
  EXPECT_EQ(alttree.getRoot().getHash(), A451->getHash());
  assertTreeTips(alttree, {A504});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A500. all tips that start at 500 should remain.
TEST_F(AltBlockFinalization, FinalizeA500) {
  // save state
  save(alttree);

  auto *A500 = A504->getAncestor(500);
  alttree.finalizeBlock(*A500);
  assertAllPreviousBlocksFinalized(A500);
  ASSERT_EQ(deallocated, 451);
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
  // save state
  save(alttree);

  Chain<BlockIndex<AltBlock>> chain = alttree.getBestChain();
  for (auto *index : chain) {
    alttree.finalizeBlock(*index);
    assertAllPreviousBlocksFinalized(index);
  }
  ASSERT_TRUE(alttree.setState(alttree.getBestChain().tip()->getHash(), state))
      << state.toString();

  assertTreeTips(alttree, {alttree.getBestChain().tip()});
  assertTreesHaveNoOrphans(alttree);
  ASSERT_EQ(deallocated, 462);
}
