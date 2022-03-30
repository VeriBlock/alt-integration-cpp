// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/mempool_fixture.hpp>
#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

struct AltBlockFinalization2 : public MemPoolFixture {
  BlockIndex<AltBlock> *tip = nullptr;
  size_t totalBlocks = 0;

  void SetUp() override {
    altparam.mMaxReorgBlocks = 500;
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

  auto *vbktip = popminer.mineVbkBlocks(10);
  fillVbkContext(altPayloads.context,
                 GetRegTestVbkBlock().getHash(),
                 vbktip->getHash(),
                 popminer.vbk());

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

  // TODO: enable these checks after vbk finalization will work
  ASSERT_EQ(alttree.vbk().getBlocks().size(), 11);

  // mine ALT block without any contained VBK context
  tip = mineAltBlocks(*alttree.getBestChain().tip(), 1);
  ASSERT_EQ(alttree.getBlocks().size(), 103);

  // save state
  save(alttree);

  alttree.finalizeBlock(*tip);
  ASSERT_EQ(alttree.getBlocks().size(), 1);
  assertTreeTips(alttree, {tip});

  // TODO: enable these checks after vbk finalization will work
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

  popminer.mineVbkBlocks(100);
  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer.vbk());
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
  // auto *vbktip = alttree.vbk().getBestChain().tip();

  // save state
  save(alttree);

  // finalize block
  alttree.finalizeBlock(*tip->pprev);

  ASSERT_EQ(alttree.getBlocks().size(), 2);
  assertTreeTips(alttree, {tip});

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  // TODO: enable these checks after vbk finalization will work
  // ASSERT_EQ(alttree.vbk().getBlocks().size(), 1);
  // assertTreeTips(alttree.vbk(), {vbktip});

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
  vbkTip = popminer.mineVbkBlocks(1, {vbkPopTx});
  auto vtb = popminer.createVTB(vbkTip->getHeader(), vbkPopTx);

  // generate ATV whuch will store in the old block
  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(chain[chain.size() - 1]));
  vbkTip = popminer.mineVbkBlocks(1, {tx});
  ATV atv = popminer.createATV(vbkTip->getHeader(), tx);

  popminer.mineVbkBlocks(100);
  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer.vbk());
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
  // auto *vbktip = alttree.vbk().getBestChain().tip();

  // finalize block
  alttree.finalizeBlock(*tip->pprev);

  assertTreeTips(alttree, {tip});

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  // TODO: enable these checks after vbk finalization will work
  // ASSERT_EQ(alttree.vbk().getBlocks().size(), 11);
  // assertTreeTips(alttree.vbk(), {vbktip});

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
  altparam.mEndorsementSettlementInterval = 10;
  altparam.mPreserveBlocksBehindFinal = 10;
  vbkparam.mEndorsementSettlementInterval = 10;
  vbkparam.mPreserveBlocksBehindFinal = 10;
  vbkparam.mOldBlocksWindow = 0;
  btcparam.mOldBlocksWindow = 0;

  // mine 10 altblocks
  mineAltBlocks(*alttree.getBestChain().tip(), 10);

  // mine btc blocks
  auto *forkBlock =
      popminer.mineBtcBlocks(btcparam.getDifficultyAdjustmentInterval() + 1);

  auto *endorsedBlock = alttree.vbk().getBestChain().tip();

  auto *chainA = popminer.mineBtcBlocks(10, *forkBlock);
  // generate vbk pop tx which will contain chainA
  auto vbkPopTx1 = generatePopTx(endorsedBlock->getHeader());
  ASSERT_EQ(vbkPopTx1.blockOfProofContext.back().getHash(), chainA->getHash());

  auto *chainB = popminer.mineBtcBlocks(12, *forkBlock);
  // generate vbk pop tx which will contain chainA
  auto vbkPopTx2 = generatePopTx(endorsedBlock->getHeader());
  ASSERT_EQ(vbkPopTx2.blockOfProofContext.back().getHash(), chainB->getHash());

  auto *vbkContaining = popminer.mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});
  auto vtb1 = popminer.createVTB(vbkContaining->getHeader(), vbkPopTx1);
  auto vtb2 = popminer.createVTB(vbkContaining->getHeader(), vbkPopTx2);

  submitVBK(vbkContaining->getHeader());
  submitVTB(vtb1);
  submitVTB(vtb2);

  auto popdata = checkedGetPop();
  ASSERT_EQ(popdata.vtbs.size(), 2);

  applyInNextBlock(popdata);

  // mine one more block on top of the block
  popdata = checkedGetPop();
  applyInNextBlock(popdata);

  save(alttree);

  tip = alttree.getBestChain().tip();
  // finalize block
  alttree.finalizeBlock(*tip->pprev->pprev);

  // check the state after finalization
  ASSERT_TRUE(alttree.setState(*tip->pprev->pprev, state)) << state.toString();
}

TEST_F(AltBlockFinalization2, FinalizeMaxBtcs) {
  altparam.mEndorsementSettlementInterval = 0;
  altparam.mPreserveBlocksBehindFinal = 0;
  vbkparam.mEndorsementSettlementInterval = 15;
  vbkparam.mPreserveBlocksBehindFinal = 15;
  vbkparam.mOldBlocksWindow = 15;
  btcparam.mOldBlocksWindow = 15;

  auto *vbkTip = popminer.mineVbkBlocks(100);
  popminer.mineBtcBlocks(100);

  const auto *endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());

  vbkTip = popminer.mineVbkBlocks(1, {vbkPopTx});

  auto vtb = popminer.createVTB(vbkTip->getHeader(), vbkPopTx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer.vbk());

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

  VBK_LOG_DEBUG("mine one more block on top of the block full of VBK payloads");
  popdata = checkedGetPop();
  applyInNextBlock(popdata);

  tip = alttree.getBestChain().tip();
  // auto *vbktip = alttree.vbk().getBestChain().tip();
  // auto *btctip = alttree.btc().getBestChain().tip();

  VBK_LOG_DEBUG("save state");
  save(alttree);

  VBK_LOG_DEBUG("finalize block");
  alttree.finalizeBlock(*tip->pprev);

  ASSERT_EQ(alttree.getBlocks().size(), 2);
  assertTreeTips(alttree, {tip});

  VBK_LOG_DEBUG("check the state after finalization");
  ASSERT_TRUE(alttree.setState(*tip->pprev, state));

  // TODO: enable these checks after vbk/btc finalization will work
  // ASSERT_EQ(
  //     alttree.vbk().getBlocks().size(),
  //     vbkparam.mOldBlocksWindow + vbkparam.mPreserveBlocksBehindFinal + 1);
  // assertTreeTips(alttree.vbk(), {vbktip});
  // ASSERT_EQ(alttree.btc().getBlocks().size(), btcparam.mOldBlocksWindow + 1);
  // assertTreeTips(alttree.btc(), {btctip});

  assertTreesHaveNoOrphans(alttree);

  ASSERT_TRUE(alttree.setState(*tip, state));
}

TEST_F(AltBlockFinalization2, comparePopScore_test) {
  // Test scenario:
  // mine 50 alt blocks
  // creating two chains: chainA (56 height), chainB(55 height)
  // active chain should be on chainA
  // saving trees to the storage
  // finalizing the tip of the chainA, so chainB should be removed
  // trying to compare chainA with the deleted chainB, chainA should win

  altparam.mEndorsementSettlementInterval = 10;
  altparam.mPreserveBlocksBehindFinal = 10;

  // mine 50 altblocks
  auto *forkPoint = mineAltBlocks(*alttree.getBestChain().tip(), 50);

  ASSERT_EQ(forkPoint->getHash(), alttree.getBestChain().tip()->getHash());

  auto *chainB = mineAltBlocks(*forkPoint, 5);
  auto *chainA = mineAltBlocks(*forkPoint, 6);

  assertTreeTips(alttree, {chainB, chainA});
  ASSERT_EQ(chainA->getHash(), alttree.getBestChain().tip()->getHash());

  save(alttree);

  // finalize block
  alttree.finalizeBlock(*chainA);

  assertTreeTips(alttree, {chainA});

  ASSERT_EQ(alttree.comparePopScore(chainA->getHash(), chainB->getHash()), 1);
}
