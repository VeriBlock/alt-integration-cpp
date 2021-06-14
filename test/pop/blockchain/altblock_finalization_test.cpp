// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <mempool_fixture.hpp>
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
  // save state
  save(alttree);

  auto *bootstrap = alttree.getBestChain().first();
  ASSERT_TRUE(alttree.finalizeBlock(*bootstrap, state));
  // unchanged
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);
  assertTreeTips(alttree, {A504, B503, C502, D502, E503, Z251});
  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization, FinalizeTip0Window) {
  // save state
  save(alttree);

  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  auto *tip = alttree.getBestChain().tip();
  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization, FinalizeUnsavedBlocks) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;

  auto *tip = alttree.getBestChain().tip();
  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);

  // save state
  save(alttree);

  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization, FinalizeUnsavedBlocksForks) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;

  auto *tip = alttree.getBestChain().tip();
  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks);

  // save state
  save(alttree);

  // mark all forks as dirty
  for (auto *tip : alttree.getTips()) {
    if (!alttree.getBestChain().contains(tip)) {
      tip->setDirty();
    }
  }

  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  // do not finilize the whole tree
  ASSERT_LT(alttree.getBlocks().size(), totalBlocks);
  ASSERT_NE(alttree.getBlocks().size(), 1);

  // save state
  save(alttree);

  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_TRUE(alttree.setState(tip->getHash(), state)) << state.toString();
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});
  assertTreesHaveNoOrphans(alttree);
}

// finalize a block A251, which has one parallel block Z251 (tip).
TEST_F(AltBlockFinalization, FinalizeA251) {
  // save state
  save(alttree);

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
  // save state
  save(alttree);

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
  // save state
  save(alttree);

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
  // save state
  save(alttree);

  Chain<BlockIndex<AltBlock>> chain = alttree.getBestChain();
  for (auto *index : chain) {
    ASSERT_TRUE(alttree.finalizeBlock(*index, state)) << index->getHeight();
  }
  ASSERT_TRUE(alttree.setState(alttree.getBestChain().tip()->getHash(), state))
      << state.toString();

  assertTreeTips(alttree, {alttree.getBestChain().tip()});
  assertTreesHaveNoOrphans(alttree);
}

struct AltBlockFinalization2 : public MemPoolFixture {
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

TEST_F(AltBlockFinalization2, FinalizeVbkTip) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mEndorsementSettlementInterval = 0;
  vbkparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mOldBlocksWindow = 0;

  PopData altPayloads;

  auto *vbktip = popminer->mineVbkBlocks(10);
  fillVbkContext(altPayloads.context,
                 GetRegTestVbkBlock().getHash(),
                 vbktip->getHash(),
                 popminer->vbk());

  AltBlock containingBlock =
      generateNextBlock(alttree.getBestChain().tip()->getHeader());
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state))
      << state.toString();
  ASSERT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  EXPECT_TRUE(state.IsValid()) << state.toString();
  vbktip = alttree.vbk().getBestChain().tip();
  ASSERT_EQ(alttree.vbk().getBlocks().size(), 11);

  // mine ALT block without any contained VBK context
  tip = mineAltBlocks(*alttree.getBestChain().tip(), 1);
  ASSERT_EQ(alttree.getBlocks().size(), 103);

  // save state
  save(alttree);

  ASSERT_TRUE(alttree.finalizeBlock(*tip, state));
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});

  ASSERT_EQ(alttree.vbk().getBlocks().size(), 1);
  assertTreeTips(alttree.vbk(), {vbktip});

  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization2, FinalizeMaxVbks) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mEndorsementSettlementInterval = 0;
  vbkparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mOldBlocksWindow = 0;

  popminer->mineVbkBlocks(100);
  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (const auto &b : context) {
    submitVBK(b);
  }

  auto popdata = checkedGetPop();
  // size of the context in the popdata should be less or equal to the
  // MAX_POPDATA_VBK
  ASSERT_LE(popdata.context.size(), MAX_POPDATA_VBK);
  ASSERT_GT(popdata.context.size(), vbkparam.mOldBlocksWindow);

  applyInNextBlock(popdata);

  // mine one more block on top of the block full of VBK payloads
  popdata = checkedGetPop();
  applyInNextBlock(popdata);

  tip = alttree.getBestChain().tip();
  auto *vbktip = alttree.vbk().getBestChain().tip();

  // save state
  save(alttree);

  // finalize block
  ASSERT_TRUE(alttree.finalizeBlock(*tip->pprev, state));

  ASSERT_EQ(alttree.getBlocks().size(), 2);
  assertTreeTips(alttree, {tip});

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  ASSERT_EQ(alttree.vbk().getBlocks().size(), 1);
  assertTreeTips(alttree.vbk(), {vbktip});

  assertTreesHaveNoOrphans(alttree);
}

TEST_F(AltBlockFinalization2, FinalizedVbkBlock) {
  altparam.mEndorsementSettlementInterval = 10;
  altparam.mPreserveBlocksBehindFinal = 10;
  vbkparam.mEndorsementSettlementInterval = 10;
  vbkparam.mPreserveBlocksBehindFinal = 10;
  vbkparam.mOldBlocksWindow = 0;

  // generate VTB which will store in the old block
  auto *vbkTip = alttree.vbk().getBestChain().tip();
  auto vbkPopTx = generatePopTx(vbkTip->getHeader());
  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx});
  auto vtb = popminer->createVTB(vbkTip->getHeader(), vbkPopTx);

  // generate ATV whuch will store in the old block
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(chain[chain.size() - 1]));
  vbkTip = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(vbkTip->getHeader(), tx);

  popminer->mineVbkBlocks(100);
  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (const auto &b : context) {
    submitVBK(b);
  }

  auto popdata = checkedGetPop();
  // size of the context in the popdata should be less or equal to the
  // MAX_POPDATA_VBK
  ASSERT_LE(popdata.context.size(), MAX_POPDATA_VBK);
  ASSERT_GT(popdata.context.size(), vbkparam.mOldBlocksWindow);

  applyInNextBlock(popdata);

  // mine one more block on top of the block full of VBK payloads
  popdata = checkedGetPop();
  applyInNextBlock(popdata);

  save(alttree);

  tip = alttree.getBestChain().tip();
  auto *vbktip = alttree.vbk().getBestChain().tip();

  // finalize block
  ASSERT_TRUE(alttree.finalizeBlock(*tip->pprev, state));

  assertTreeTips(alttree, {tip});

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  ASSERT_EQ(alttree.vbk().getBlocks().size(), 11);
  assertTreeTips(alttree.vbk(), {vbktip});

  assertTreesHaveNoOrphans(alttree);

  // insert the old vtb into the popdata
  popdata.vtbs = {vtb};
  popdata.atvs = {atv};
  ASSERT_EQ(popdata.vtbs.size(), 1);
  ASSERT_EQ(popdata.atvs.size(), 1);
  ASSERT_EQ(popdata.context.size(), 0);

  applyInNextBlock(popdata);
}

TEST_F(AltBlockFinalization2, FinalizeForkedBtcBlocks) {

  
}

TEST_F(AltBlockFinalization2, FinalizeMaxBtcs) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mEndorsementSettlementInterval = 15;
  vbkparam.mPreserveBlocksBehindFinal = 15;
  vbkparam.mOldBlocksWindow = 15;
  btcparam.mOldBlocksWindow = 15;

  auto *vbkTip = popminer->mineVbkBlocks(100);
  popminer->mineBtcBlocks(100);

  const auto *endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx});

  auto vtb = popminer->createVTB(vbkTip->getHeader(), vbkPopTx);

  // mine 10 blocks
  // mineAltBlocks(1, chain);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  for (const auto &b : context) {
    submitVBK(b);
  }
  submitVTB(vtb);

  auto popdata = checkedGetPop();
  // size of the context in the popdata should be less or equal to the
  // MAX_POPDATA_VBK
  ASSERT_LE(popdata.context.size(), MAX_POPDATA_VBK);
  ASSERT_GT(popdata.context.size(), vbkparam.mOldBlocksWindow);
  ASSERT_EQ(popdata.vtbs.size(), 1);
  ASSERT_LE(popdata.vtbs[0].transaction.blockOfProofContext.size(),
            MAX_BTC_BLOCKS_IN_VBKPOPTX);
  ASSERT_GT(popdata.vtbs[0].transaction.blockOfProofContext.size(),
            btcparam.mOldBlocksWindow);

  applyInNextBlock(popdata);

  // mine one more block on top of the block full of VBK payloads
  popdata = checkedGetPop();
  applyInNextBlock(popdata);

  tip = alttree.getBestChain().tip();
  auto *vbktip = alttree.vbk().getBestChain().tip();
  auto *btctip = alttree.btc().getBestChain().tip();

  // save state
  save(alttree);

  // finalize block
  ASSERT_TRUE(alttree.finalizeBlock(*tip->pprev, state));

  ASSERT_EQ(alttree.getBlocks().size(), 2);
  assertTreeTips(alttree, {tip});

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  ASSERT_EQ(
      alttree.vbk().getBlocks().size(),
      vbkparam.mOldBlocksWindow + vbkparam.mPreserveBlocksBehindFinal + 1);
  assertTreeTips(alttree.vbk(), {vbktip});

  ASSERT_EQ(alttree.btc().getBlocks().size(), btcparam.mOldBlocksWindow + 1);
  assertTreeTips(alttree.btc(), {btctip});

  assertTreesHaveNoOrphans(alttree);

  ASSERT_TRUE(alttree.setState(*tip, state));
}