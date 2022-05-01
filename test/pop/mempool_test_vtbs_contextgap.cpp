// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "pop/util/mempool_fixture.hpp"

TEST_F(MemPoolFixture, mempool_vtbs_contextgap_gap) {
  // When only VBK blocks are added but not VTBs, we may end up in a situation
  // when we no longer can advance BTC context, due to all VTBs added into
  // "finalized" (tip-2000) VBK blocks. Issue:
  // https://github.com/VeriBlock/alt-integration-cpp/issues/894

  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[6];
  AltBlock endorsedBlock2 = chain[5];

  // mine 20 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(20);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);

  // generate two VTBs
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer.mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer.mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer.createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer.createVTB(vbkTip->getHeader(), vbkPopTx2);

  // generate two ATVs
  const auto& tx1 = popminer.createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock1), Coin(1000));
  const auto& tx2 = popminer.createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock2), Coin(1000));
  const auto& block = popminer.mineVbkBlocks(1, {tx1, tx2})->getHeader();
  auto pd1 = popminer.createPopDataEndorsingAltBlock(
      block, tx1, getLastKnownVbkBlock());
  ATV& atv1 = pd1.atvs.at(0);
  auto pd2 = popminer.createPopDataEndorsingAltBlock(
      block, tx2, getLastKnownVbkBlock());
  ATV& atv2 = pd2.atvs.at(0);

  // generate VBK fork block
  auto* fork_block = popminer.mineVbkBlocks(1, *vbkTip);

  // mine vbk blocks more than "old block" window
  vbkTip = popminer.mineVbkBlocks(vbkparam.getOldBlocksWindow() + 5);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer.vbk());
  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    ASSERT_TRUE(mempool.submit(*it, true, state));
  }

  state.reset();

  while (alttree.vbk().getBestChain().tip()->getHeight() !=
         vbkTip->getHeight()) {
    auto pop_data = mempool.generatePopData();
    mineAltBlocks(1, chain, false, false);
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop_data));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
  }

  ASSERT_FALSE(mempool.submit(atv1, true, state));
  ASSERT_FALSE(mempool.submit(atv2, true, state));
  ASSERT_FALSE(mempool.submit(fork_block->getHeader(), true, state));

  ASSERT_TRUE(mempool.submit(vtb1, true, state));
  ASSERT_TRUE(mempool.submit(vtb2, true, state));
}
