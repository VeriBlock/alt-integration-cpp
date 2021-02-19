// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "mempool_fixture.hpp"

TEST_F(MemPoolFixture, getPop_scenario_1) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  ASSERT_NE(vtbs[0].getId(), vtbs[1].getId());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->getATVs(*block)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);
  for (const auto& vtb : vtbs) {
    submitVTB(vtb);
  }
  for (const auto& b : context) {
    submitVBK(b);
  }
  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHeight(), 10);
  PopData pop = checkedGetPop();
  EXPECT_EQ(pop.vtbs.size(), 2);
  applyInNextBlock(pop);
}

TEST_F(MemPoolFixture, getPop_scenario_2) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx1});
  ASSERT_EQ(popminer->vbkPayloads.at(containingVbkBlock1->getHash()).size(), 1);
  VTB vtb1 = popminer->vbkPayloads.at(containingVbkBlock1->getHash()).at(0);

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1, {vbkPopTx2});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->getATVs(*block)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);

  submitVTB(vtb1);

  submitVTB(vtb2);

  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 2);

  applyInNextBlock(v_popData);
}

TEST_F(MemPoolFixture, getPop_scenario_3) {
  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->getATVs(*block)[0];

  submitATV(atv);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.context.size(), 0);
  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_4) {
  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->getATVs(*block)[0];

  submitATV(atv);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 0);

  applyInNextBlock(v_popData);
}

TEST_F(MemPoolFixture, getPop_scenario_5) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx1});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->getATVs(*block1)[0];

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1, {vbkPopTx2});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  auto* block2 = popminer->mineVbkBlocks(1, {tx2});
  ATV atv2 = popminer->getATVs(*block2)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  {
    submitATV(atv1);
    submitATV(atv2);
    submitVTB(vtb2);
    submitVTB(vtb1);
    for (const auto& b : context) {
      submitVBK(b);
    }

    ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
    PopData v_popData = checkedGetPop();

    ASSERT_EQ(v_popData.context.size(), 177);
    ASSERT_EQ(v_popData.vtbs.size(), 2);
    ASSERT_EQ(v_popData.atvs.size(), 2);

    EXPECT_EQ(v_popData.atvs.at(0), atv1);
    EXPECT_EQ(v_popData.atvs.at(1), atv2);
    EXPECT_EQ(v_popData.vtbs.at(0), vtb1);
    EXPECT_EQ(v_popData.vtbs.at(1), vtb2);

    applyInNextBlock(v_popData);
  }

  mempool->clear();
  removeLastAltBlock();

  {
    // different order
    submitATV(atv2);
    submitATV(atv1);
    submitVTB(vtb2);
    submitVTB(vtb1);
    for (const auto& b : context) {
      submitVBK(b);
    }

    ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
    PopData v_popData = checkedGetPop();

    ASSERT_EQ(v_popData.context.size(), 177);
    ASSERT_EQ(v_popData.vtbs.size(), 2);
    ASSERT_EQ(v_popData.atvs.size(), 2);

    EXPECT_EQ(v_popData.atvs.at(0), atv1);
    EXPECT_EQ(v_popData.atvs.at(1), atv2);
    EXPECT_EQ(v_popData.vtbs.at(0), vtb1);
    EXPECT_EQ(v_popData.vtbs.at(1), vtb2);

    applyInNextBlock(v_popData);
  }

  mempool->clear();
  removeLastAltBlock();
  context.erase(--context.end());

  {
    // No ATV2
    submitATV(atv1);
    submitVTB(vtb1);
    submitVTB(vtb2);
    for (const auto& b : context) {
      submitVBK(b);
    }

    ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
    PopData v_popData = checkedGetPop();

    ASSERT_EQ(v_popData.context.size(), 176);
    ASSERT_EQ(v_popData.vtbs.size(), 2);
    ASSERT_EQ(v_popData.atvs.size(), 1);

    EXPECT_EQ(v_popData.atvs.at(0), atv1);
    EXPECT_EQ(v_popData.vtbs.at(0), vtb1);
    EXPECT_EQ(v_popData.vtbs.at(1), vtb2);

    applyInNextBlock(v_popData);
  }
}

TEST_F(MemPoolFixture, getPop_scenario_6) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);

  auto vbkPopTx = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  // generate VTB with the duplicate
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({vbkPopTx});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer->vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // add containing block without payloads into vbk tree
  EXPECT_TRUE(popminer->vbk().acceptBlock(containingVbkBlock, state));

  // Create VTB
  VTB vtb2;
  vtb2.transaction = vbkPopTx;
  vtb2.merklePath.treeIndex = treeIndex;
  vtb2.merklePath.index = 0;
  vtb2.merklePath.subject = hashes[0];
  vtb2.merklePath.layers = mtree.getMerklePathLayers(0);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer->btc().getParams(), popminer->getVbkParams()));

  EXPECT_NE(vtb1.containingBlock, vtb2.containingBlock);
  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  EXPECT_EQ(E1.blockOfProof, E2.blockOfProof);
  EXPECT_EQ(E1.endorsedHash, E2.endorsedHash);
  EXPECT_NE(E1.containingHash, E2.containingHash);
  EXPECT_NE(E1.id, E2.id);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->getATVs(*block1)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv1);
  submitVTB(vtb1);
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.context.size(), 122);
  EXPECT_EQ(v_popData.vtbs.size(), 2);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.vtbs.at(0), vtb1);
  EXPECT_EQ(v_popData.vtbs.at(1), vtb2);

  applyInNextBlock(v_popData);
}

// We expect that ATV duplicate will not be added
TEST_F(MemPoolFixture, getPop_scenario_7) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->getATVs(*block1)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv1);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();
  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.atvs.at(0), atv1);
  applyInNextBlock(v_popData);

  mempool->removeAll(v_popData);
  submitATV(atv1);
  ASSERT_EQ(mempool->getMap<ATV>().size(), 0);
  ASSERT_EQ(mempool->getInFlightMap<ATV>().size(), 1);
}

TEST_F(MemPoolFixture, unimplemented_getPop_scenario_8) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);

  auto vbkPopTx = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->getATVs(*block1)[0];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv1);
  submitVTB(vtb1);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 1);
  EXPECT_EQ(v_popData.vtbs.at(0), vtb1);

  applyInNextBlock(v_popData);

  // remove payloads from the mempool
  mempool->removeAll(v_popData);

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  // generate VTB with the duplicate
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({vbkPopTx});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer->vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // add containing block without payloads into vbk tree
  EXPECT_TRUE(popminer->vbk().acceptBlock(containingVbkBlock, state));

  // Create VTV
  VTB vtb2;
  vtb2.transaction = vbkPopTx;
  vtb2.merklePath.treeIndex = treeIndex;
  vtb2.merklePath.index = 0;
  vtb2.merklePath.subject = hashes[0];
  vtb2.merklePath.layers = mtree.getMerklePathLayers(0);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer->btc().getParams(), popminer->getVbkParams()));

  EXPECT_NE(vtb1.containingBlock, vtb2.containingBlock);
  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  EXPECT_EQ(E1.blockOfProof, E2.blockOfProof);
  EXPECT_EQ(E1.endorsedHash, E2.endorsedHash);
  EXPECT_NE(E1.containingHash, E2.containingHash);
  EXPECT_NE(E1.id, E2.id);

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  auto* block2 = popminer->mineVbkBlocks(1, {tx2});
  ATV atv2 = popminer->getATVs(*block2)[0];
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv2);
  // mempool should discard such transactions
  // EXPECT_FALSE(mempool->submit(vtb2, state)) << state.toString();
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 1);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.context.size(), 56);
}
// This test scenrio tests filter payloads duplicates in the same altblock chain
TEST_F(MemPoolFixture, getPop_scenario_9) {
  size_t totalBlocks = 0, totalAtvs = 0, totalVtbs = 0;
  mempool->onAccepted<VbkBlock>([&](const VbkBlock&) { totalBlocks++; });
  mempool->onAccepted<VTB>([&](const VTB&) { totalVtbs++; });
  mempool->onAccepted<ATV>([&](const ATV&) { totalAtvs++; });
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->getATVs(*block1)[0];
  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  for (const auto& b : context) {
    submitVBK(b);
  }
  submitATV(atv1);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.context.size(), 66);
  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.atvs.at(0), atv1);

  applyInNextBlock(v_popData);

  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block2 = popminer->mineVbkBlocks(1, {tx2});
  ATV atv2 = popminer->getATVs(*block2)[0];
  context.clear();
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  for (const auto& b : context) {
    submitVBK(b);
  }
  submitATV(atv2);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.atvs.at(0), atv2);
  EXPECT_EQ(v_popData.context.size(), 1);
  EXPECT_EQ(v_popData.vtbs.size(), 0);

  applyInNextBlock(v_popData);

  ASSERT_EQ(totalBlocks, 67);
  ASSERT_EQ(totalVtbs, 0);
  ASSERT_EQ(totalAtvs, 2);
}

// This test scenrio tests the possible context gap in case that all payloads
// contain in the same PopData which bigger than maxPopDataSize
TEST_F(MemPoolFixture, getPop_scenario_11) {
  altparam.mMaxPopDataSize = 10000;
  auto* vbkTip = popminer->mineVbkBlocks(65);

  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  size_t vtbs_amount = 100;
  std::vector<VbkPopTx> transactions(vtbs_amount);
  for (size_t i = 0; i < vtbs_amount; ++i) {
    popminer->mineBtcBlocks(10);
    transactions[i] = generatePopTx(endorsedVbkBlock1->getHeader());
  }

  vbkTip = popminer->mineVbkBlocks(1, transactions);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (const auto& blk : context) {
    submitVBK(blk);
  }

  ASSERT_EQ(vtbs.size(), vtbs_amount);

  for (const auto& vtb : vtbs) {
    submitVTB(vtb);
  }

  ASSERT_EQ(mempool->getMap<VTB>().size(), vtbs_amount);

  PopData pop = checkedGetPop();

  EXPECT_LT(pop.vtbs.size(), vtbs_amount);
  applyInNextBlock(pop);
}

TEST_F(MemPoolFixture, getPop_scenario_12) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  size_t vbkblocks_count = 100;

  std::vector<VbkBlock> vbk_blocks;
  for (size_t i = 0; i < vbkblocks_count; ++i) {
    VbkBlock block =
        vbk_miner.createNextBlock(*popminer->vbk().getBestChain().tip());
    // add containing block without payloads into vbk tree
    EXPECT_TRUE(popminer->vbk().acceptBlock(block, state));
    vbk_blocks.push_back(block);
  }

  EXPECT_EQ(vbk_blocks.size(), vbkblocks_count);
  EXPECT_TRUE(vbk_blocks.back().getHash() ==
              popminer->vbk().getBestChain().tip()->getHash());

  for (size_t i = 0; i < vbk_blocks.size(); ++i) {
    submitVBK(vbk_blocks[i]);
  }

  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.context.size(), vbkblocks_count);
  EXPECT_EQ(popData.vtbs.size(), 0);
  EXPECT_EQ(popData.atvs.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_13) {
  popminer->mineBtcBlocks(100);
  auto* vbkTip = popminer->mineVbkBlocks(54);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock->getHash()].size(), 2);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock->getHash()][0];
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock->getHash()][1];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  EXPECT_EQ(vtb1.containingBlock, vtb2.containingBlock);

  submitVTB(vtb1);

  for (const auto& b : context) {
    submitVBK(b);
  }

  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.atvs.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 1);
  EXPECT_TRUE(!popData.context.empty());

  applyInNextBlock(popData);
  mempool->removeAll(popData);

  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  popData = checkedGetPop();

  EXPECT_EQ(popData.atvs.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 1);
  EXPECT_TRUE(popData.context.empty());

  applyInNextBlock(popData);
}

// Test scenrio when we have added into the mempool some statefully invalid
// payloads (duplicates) and they should be removed from the inFlight storage
// later
TEST_F(MemPoolFixture, getPop_scenario_14) {
  popminer->mineBtcBlocks(100);
  auto* vbkTip = popminer->mineVbkBlocks(54);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock->getHash()].size(), 2);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock->getHash()][0];
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock->getHash()][1];

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitVTB(vtb1);
  submitVTB(vtb2);

  for (const auto& b : context) {
    submitVBK(b);
  }

  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.atvs.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_TRUE(!popData.context.empty());

  applyInNextBlock(popData);
  mempool->removeAll(popData);

  // again submit payloads into the mempool
  submitVTB(vtb1);
  submitVTB(vtb2);

  EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
  EXPECT_EQ(mempool->getInFlightMap<VTB>().size(), 2);

  mempool->cleanUp();

  EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
  EXPECT_EQ(mempool->getInFlightMap<VTB>().size(), 0);
}
