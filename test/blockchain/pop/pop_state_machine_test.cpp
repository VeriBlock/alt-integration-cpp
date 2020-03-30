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

TEST(PopStateMachine, unapplyAndApply_test) {
  srand(0);
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  ValidationState state;

  PayloadsRepositoryInmem<VTB> payloadsRepo;

  MockMiner apm;

  BtcTree btcTree(apm.getBtcParams());
  ASSERT_TRUE(btcTree.bootstrapWithGenesis(state));

  PopStateMachine<BtcTree, BlockIndex<VbkBlock>, VbkChainParams> stateMachine(
      btcTree,
      apm.vbk().getBestChain().tip(),
      apm.getVbkParams(),
      payloadsRepo);

  apm.mineBtcBlocks(10);

  auto* vbkForkpoint = apm.mineVbkBlocks(39);
  ASSERT_EQ(vbkForkpoint->getHash(), apm.vbk().getBestChain().tip()->getHash());

  auto* B10 = vbkForkpoint->getAncestor(10);
  auto Btx1 = apm.createBtcTxEndorsingVbkBlock(B10->header);
  auto Bbtccontaining1 = apm.mineBtcBlocks(1);
  ASSERT_TRUE(apm.btc().getBestChain().contains(Bbtccontaining1));

  apm.createVbkPopTxEndorsingVbkBlock(
      Bbtccontaining1->header,
      Btx1,
      B10->header,
      apm.getBtcParams().getGenesisBlock().getHash());

  auto* vbkTip1 = apm.mineVbkBlocks(*vbkForkpoint, 1);

  ASSERT_EQ(vbkTip1->containingPayloads.size(), 1);

  auto btcTip1 = *apm.btc().getBestChain().tip();

  std::vector<VTB> vtbs;
  apm.getGeneratedVTBs(*vbkTip1, vtbs);
  ASSERT_EQ(vtbs.size(), vbkTip1->containingPayloads.size());

  payloadsRepo.put(vtbs[0]);

  vbkTip1 = apm.mineVbkBlocks(*vbkTip1, 20);

  EXPECT_TRUE(stateMachine.unapplyAndApply(*vbkTip1, state));

  EXPECT_EQ(btcTree.getBestChain().tip()->getHash(), btcTip1.getHash());

  // mine BtcBlocks 115
  apm.mineBtcBlocks(115);

  auto Btx2 = apm.createBtcTxEndorsingVbkBlock(B10->header);
  auto Bbtccontaining2 = apm.mineBtcBlocks(1);
  ASSERT_TRUE(apm.btc().getBestChain().contains(Bbtccontaining2));

  apm.createVbkPopTxEndorsingVbkBlock(
      Bbtccontaining2->header,
      Btx2,
      B10->header,
      apm.getBtcParams().getGenesisBlock().getHash());

  auto* vbkTip2 = apm.mineVbkBlocks(*vbkForkpoint, 1);

  ASSERT_EQ(vbkTip2->containingPayloads.size(), 1);

  auto btcTip2 = *apm.btc().getBestChain().tip();

  vtbs.clear();
  apm.getGeneratedVTBs(*vbkTip2, vtbs);
  ASSERT_EQ(vtbs.size(), vbkTip2->containingPayloads.size());

  payloadsRepo.put(vtbs[0]);

  vbkTip2 = apm.mineVbkBlocks(*vbkTip2, 20);

  EXPECT_TRUE(stateMachine.unapplyAndApply(*vbkTip2, state));

  EXPECT_EQ(btcTree.getBestChain().tip()->getHash(), btcTip2.getHash());

  for (auto *workBlock1 = vbkTip1, *workBlock2 = vbkTip2;
       workBlock1 != vbkForkpoint && workBlock2 != vbkForkpoint;) {
    if (workBlock1 != vbkForkpoint) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*workBlock1, state));

      EXPECT_EQ(btcTip1.getHash(), btcTree.getBestChain().tip()->getHash());
      workBlock1 = workBlock1->pprev;
    }

    if (workBlock2 != vbkForkpoint) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*workBlock2, state));

      EXPECT_EQ(btcTip2.getHash(), btcTree.getBestChain().tip()->getHash());
      workBlock2 = workBlock2->pprev;
    }
  }
}
