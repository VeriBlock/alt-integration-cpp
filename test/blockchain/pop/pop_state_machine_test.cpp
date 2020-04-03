#include <gtest/gtest.h>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/pop/pop_state_machine.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/payloads_repository_inmem.hpp"

using namespace altintegration;

namespace altintegration {}  // namespace altintegration

TEST(PopStateMachine, unapplyAndApply_test) {
  srand(0);
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  ValidationState state;

  MockMiner apm;

  /// start with empty tree
  BtcTree btcTree(apm.getBtcParams());
  ASSERT_TRUE(btcTree.bootstrapWithGenesis(state));
  PopStateMachine<BtcTree, BlockIndex<VbkBlock>, VbkChainParams> stateMachine(
      btcTree, apm.vbk().getBestChain().tip(), apm.getVbkParams());

  // APM BTC: 11 blocks
  // local  : 1  block
  apm.mineBtcBlocks(10);
  ASSERT_EQ(btcTree.getBestChain().tip()->height, 0);
  ASSERT_EQ(apm.btc().getBestChain().tip()->height, 10);

  // APM VBK: 40 blocks
  auto* vbkTip = apm.mineVbkBlocks(40);
  ASSERT_EQ(vbkTip->getHash(), apm.vbk().getBestChain().tip()->getHash());

  // endorse VBK block 10
  auto* B10 = vbkTip->getAncestor(10);
  auto Btx1 = apm.createBtcTxEndorsingVbkBlock(B10->header);
  // store endorsement in APM BTC: 11
  auto Bbtccontaining1 = apm.mineBtcBlocks(1);
  ASSERT_TRUE(apm.btc().getBestChain().contains(Bbtccontaining1));
  ASSERT_EQ(Bbtccontaining1->height, 11);

  // store endorsement in VBK
  apm.createVbkPopTxEndorsingVbkBlock(
      Bbtccontaining1->header,
      Btx1,
      B10->header,
      apm.getBtcParams().getGenesisBlock().getHash());

  // in block 41
  auto* vbkTip1 = apm.mineVbkBlocks(*vbkTip, 1);
  ASSERT_EQ(apm.vbkContext.size(), 1);
  auto it = apm.vbkContext.find(vbkTip1->getHash());
  ASSERT_NE(it, apm.vbkContext.end());

  // HACK: manually add missing context to VBK tip
  addContextToBlockIndex(*vbkTip1, it->second[0], btcTree);
  ASSERT_EQ(vbkTip1->height, 41);
  ASSERT_EQ(apm.vbk().getBestChain().tip(), vbkTip1);
  ASSERT_TRUE(state.IsValid());

  // add 20 blocks on top of current vbk tip
  vbkTip1 = apm.mineVbkBlocks(*vbkTip1, 20);
  ASSERT_EQ(apm.vbk().getBestChain().tip(), vbkTip1);
  ASSERT_EQ(apm.vbk().getBestChain().tip()->height, 61);

  // change LOCAL BTC view to same as VBK TIP1.
  // it should have 12 blocks, last is 11th
  ASSERT_TRUE(stateMachine.unapplyAndApply(*vbkTip1, state));
  ASSERT_EQ(stateMachine.index(), vbkTip1);

  // LOCAL BTC and APM BTC tips are same
  ASSERT_EQ(*btcTree.getBestChain().tip(), *apm.btc().getBestChain().tip());

  //
  auto btc1tip = apm.btc().getBestChain().tip();
  EXPECT_EQ(btcTree.getBestChain().tip()->getHash(), btc1tip->getHash());

  apm.mineBtcBlocks(115);

  auto Btx2 = apm.createBtcTxEndorsingVbkBlock(B10->header);
  auto Bbtccontaining2 = apm.mineBtcBlocks(1);
  ASSERT_TRUE(apm.btc().getBestChain().contains(Bbtccontaining2));

  apm.createVbkPopTxEndorsingVbkBlock(
      Bbtccontaining2->header,
      Btx2,
      B10->header,
      apm.getBtcParams().getGenesisBlock().getHash());

  auto* vbkTip2 = apm.mineVbkBlocks(*vbkTip, 1);
  ASSERT_EQ(apm.vbkContext.size(), 2);
  it = apm.vbkContext.find(vbkTip2->getHash());
  ASSERT_NE(it, apm.vbkContext.end());

  // HACK: manually add missing context to VBK tip
  BtcTree tempBtcTree(apm.getBtcParams());
  ASSERT_TRUE(tempBtcTree.bootstrapWithGenesis(state));
  addContextToBlockIndex(*vbkTip2, it->second[0], tempBtcTree);

  auto btcTip2 = *apm.btc().getBestChain().tip();

  vbkTip2 = apm.mineVbkBlocks(*vbkTip2, 20);

  EXPECT_TRUE(stateMachine.unapplyAndApply(*vbkTip2, state));

  EXPECT_EQ(*btcTree.getBestChain().tip(), btcTip2);

  for (auto *workBlock1 = vbkTip1, *workBlock2 = vbkTip2;
       workBlock1 != vbkTip && workBlock2 != vbkTip;) {
    if (workBlock1 != vbkTip) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*workBlock1, state));

      EXPECT_EQ(*btc1tip, *btcTree.getBestChain().tip());
      workBlock1 = workBlock1->pprev;
    }

    if (workBlock2 != vbkTip) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*workBlock2, state));

      EXPECT_EQ(btcTip2, *btcTree.getBestChain().tip());
      workBlock2 = workBlock2->pprev;
    }
  }
}
