// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "pop/util/mempool_fixture.hpp"

TEST_F(MemPoolFixture, removeAll_test1) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);

  submitVTB(vtb1);
  submitVTB(vtb2);

  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // do the same to show that from mempool do not remove payloads
  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // remove from mempool
  mempool->removeAll(popData);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));

  EXPECT_TRUE(mempool->getMap<ATV>().empty());
  EXPECT_TRUE(mempool->getMap<VTB>().empty());
  EXPECT_EQ(mempool->getMap<VbkBlock>().size(), 0);

  popData = checkedGetPop();
  EXPECT_EQ(popData.context.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 0);
  EXPECT_EQ(popData.atvs.size(), 0);
}

TEST_F(MemPoolFixture, removeAll_test2) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);
  submitVTB(vtb1);
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }
  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // modify popData to not remove all payloads
  popData.atvs.clear();

  mempool->removeAll(popData);

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));

  EXPECT_FALSE(mempool->getMap<ATV>().empty());
  EXPECT_TRUE(mempool->getMap<VTB>().empty());
  EXPECT_FALSE(mempool->getMap<VbkBlock>().empty());
  ASSERT_EQ(mempool->getMap<VbkBlock>().size(), 1);
  ASSERT_EQ(mempool->getMap<ATV>().size(), 1);
  EXPECT_EQ(mempool->getMap<VbkBlock>().begin()->second->getHash(),
            mempool->getMap<ATV>().begin()->second->blockOfProof.getHash());
}

TEST_F(MemPoolFixture, removeAll_test3) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);
  submitVTB(vtb1);
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData popData = checkedGetPop();
  applyInNextBlock(popData);

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);
  EXPECT_FALSE(popData.context.empty());

  size_t prev_size = popData.context.size();
  size_t removed = 0;
  // remove from popData payloads containing vbk blocks
  for (auto it = popData.context.begin(); it != popData.context.end();) {
    if (it->getHash() == popData.atvs[0].blockOfProof.getHash() ||
        it->getHash() == popData.vtbs[0].containingBlock.getHash() ||
        it->getHash() == popData.vtbs[1].containingBlock.getHash()) {
      it = popData.context.erase(it);
      ++removed;
      continue;
    }
    ++it;
  }

  EXPECT_EQ(popData.context.size(), prev_size - removed);

  mempool->removeAll(popData);

  ASSERT_TRUE(mempool->getMap<ATV>().empty());
  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());
}

TEST_F(MemPoolFixture, removeAll_test4) {
  // mine 15 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(15);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);
  submitVTB(vtb1);
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData popData = checkedGetPop();
  applyInNextBlock(popData);

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);
  EXPECT_FALSE(popData.context.empty());

  mempool->removeAll(popData);

  // add same ATV again
  // TODO: we do not return false value while payloads statefully incorect

  submitATV(atv);
  ASSERT_EQ(mempool->getMap<ATV>().size(), 0);
  ASSERT_EQ(mempool->getInFlightMap<ATV>().size(), 1);
  // ASSERT_EQ(state.GetPath(),
  // "pop-mempool-submit-atv-stateful+atv-duplicate"); state.clear();

  ASSERT_TRUE(mempool->getMap<ATV>().empty());
  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());

  popminer->mineVbkBlocks(300);
  auto vbkPopTx =
      generatePopTx(popminer->vbk().getBestChain().tip()->getHeader());
  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx});
  auto vtb = popminer->createVTB(vbkTip->getHeader(), vbkPopTx);

  context.clear();
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitVTB(vtb);
  for (const auto& b : context) {
    submitVBK(b);
  }

  do {
    popData = checkedGetPop();
    applyInNextBlock(popData);
    mempool->removeAll(popData);
  } while (!popData.empty());

  ASSERT_TRUE(mempool->getMap<ATV>().empty()) << mempool->getMap<ATV>().size();
  ASSERT_TRUE(mempool->getMap<VTB>().empty()) << mempool->getMap<VTB>().size();
  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty())
      << mempool->getMap<VbkBlock>().size();
}

TEST_F(MemPoolFixture, removed_payloads_cache_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitATV(atv);
  submitVTB(vtb1);
  submitVTB(vtb2);
  for (const auto& b : context) {
    submitVBK(b);
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));
  PopData popData = mempool->generatePopData();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_FALSE(popData.context.empty());
  EXPECT_EQ(popData.atvs.at(0), atv);

  applyInNextBlock(popData);
  mempool->removeAll(popData);

  popData = mempool->generatePopData();

  EXPECT_TRUE(popData.vtbs.empty());
  EXPECT_TRUE(popData.atvs.empty());
  EXPECT_TRUE(popData.context.empty());

  // insert the same payloads into the mempool
  submitATV(atv);
  EXPECT_EQ(mempool->getMap<ATV>().size(), 0);
  submitVTB(vtb1);
  EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
  submitVTB(vtb2);
  EXPECT_EQ(mempool->getMap<VTB>().size(), 0);

  popData = mempool->generatePopData();
  EXPECT_TRUE(popData.vtbs.empty());
  EXPECT_TRUE(popData.atvs.empty());
  EXPECT_TRUE(popData.context.empty());
}

// test that mempool would reject not continuous vbk blocks
TEST_F(MemPoolFixture, submit_vbk_blocks) {
  // miner 65 vbk blocks
  popminer->mineVbkBlocks(65);

  ASSERT_EQ(popminer->vbk().getBestChain().tip()->getHeight(), 65);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  ASSERT_EQ(context.size(), 65);

  // validate that we have valid vbk block context
  for (const auto& b : context) {
    submitVBK(b);
  }

  // clean mempool
  mempool->clear();
  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());

  ASSERT_EQ(context.back().getHeight(), 65);
  ASSERT_EQ((++context.rbegin())->getHeight(), 64);

  // corrupt continuity of the vbk blocks
  context.erase(--(--context.end()));
  ASSERT_EQ(context.size(), 64);
  ASSERT_EQ(context.back().getHeight(), 65);
  ASSERT_EQ((++context.rbegin())->getHeight(), 63);

  for (size_t i = 0; i < context.size() - 1; ++i) {
    submitVBK(context[i]);
  }

  submitVBK(context.back());
  EXPECT_EQ(mempool->getMap<VbkBlock>().size(), context.size() - 1);
  EXPECT_EQ(mempool->getInFlightMap<VbkBlock>().size(), 1);
}

TEST_F(MemPoolFixture, submit_deprecated_payloads) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  popminer->mineVbkBlocks(
      popminer->vbk().getParams().getEndorsementSettlementInterval());

  std::vector<VbkPopTx> txes;
  txes.push_back(generatePopTx(endorsedVbkBlock1->getHeader()));
  popminer->mineBtcBlocks(100);
  txes.push_back(generatePopTx(endorsedVbkBlock2->getHeader()));

  // generate vtbs with the deprecated endorsements
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>(txes);
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingBlock = vbk_miner.createNextBlock(
      *popminer->vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());
  EXPECT_TRUE(popminer->vbk().acceptBlockHeader(containingBlock, state));

  // map VbkPopTx -> VTB
  std::vector<VTB> vtbs;
  vtbs.reserve(txes.size());
  int32_t index = 0;
  std::transform(txes.begin(),
                 txes.end(),
                 std::back_inserter(vtbs),
                 [&](const VbkPopTx& tx) -> VTB {
                   VTB vtb;
                   vtb.transaction = tx;
                   vtb.merklePath.treeIndex = treeIndex;
                   vtb.merklePath.index = index;
                   vtb.merklePath.subject = hashes[index];
                   vtb.merklePath.layers = mtree.getMerklePathLayers(index);
                   vtb.containingBlock = containingBlock;
                   index++;

                   return vtb;
                 });

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  mineAltBlocks(alttree.getParams().getPayoutParams().getPopPayoutDelay(),
                chain);

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);

  // insert the same payloads into the mempool
  submitATV(atv);
  EXPECT_EQ(mempool->getMap<ATV>().size(), 0);
  for (const auto& vtb : vtbs) {
    EXPECT_TRUE(
        checkVTB(vtb, state, popminer->btcParams(), popminer->vbkParams()));
    submitVTB(vtb);
    EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
  }
}

TEST_F(MemPoolFixture, BtcBlockReferencedTooEarly) {
  // scenario:
  // [1..(10)] = BTC context from BTC1 to BTC9. BTC10 is block of proof
  // VBK8  contains VTB0 with BTC[1..(6)]
  // VBK9  contains VTB1 with BTC[8..(9)]
  // VBK10 contains VTB2 with BTC[7..(11)]
  // send all VBK blocks first -> expect them to connect
  // send VTB0 -> expect it to connect
  // send VTB2 -> expect it to connect
  // send VTB1 -> expect it to NOT connect in mempool, because VTB1 context
  // starts at 8, and previous VBK blocks bring only context up to 6, block 7 is
  // missing. Error should be "btc block referenced too early".

  /// First, prepare VTB0, VTB1, VTB2
  auto vbkendorsed = popminer->mineVbkBlocks(7);
  auto btctx0 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());
  auto btctx1 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());
  auto btctx2 =
      popminer->createBtcTxEndorsingVbkBlock(vbkendorsed->getHeader());

  // mine BTC1..5
  popminer->mineBtcBlocks(5);
  // mine VTB0 block of proof
  auto btc6 = popminer->mineBtcBlocks(1, {btctx0});
  ASSERT_EQ(btc6->getHeight(), 6);
  auto btc7 = popminer->mineBtcBlocks(1);
  // BTC8
  popminer->mineBtcBlocks(1);
  // mine VTB1 block of proof
  auto btc9 = popminer->mineBtcBlocks(1, {btctx1});
  ASSERT_EQ(btc9->getHeight(), 9);
  // BTC10
  popminer->mineBtcBlocks(1);
  // mine VTB2 block of proof
  auto btc11 = popminer->mineBtcBlocks(1, {btctx2});
  ASSERT_EQ(btc11->getHeight(), 11);

  // create POP TX for VTB0
  auto poptx0 =
      popminer->createVbkPopTxEndorsingVbkBlock(btc6->getHeader(),
                                                btctx0,
                                                vbkendorsed->getHeader(),
                                                // equals to genesis
                                                getLastKnownBtcBlock());
  // create POP TX for VTB1
  auto poptx1 =
      popminer->createVbkPopTxEndorsingVbkBlock(btc9->getHeader(),
                                                btctx1,
                                                vbkendorsed->getHeader(),
                                                // context starts at 8
                                                btc7->getHash());
  // create POP TX for VTB2
  auto poptx2 =
      popminer->createVbkPopTxEndorsingVbkBlock(btc11->getHeader(),
                                                btctx2,
                                                vbkendorsed->getHeader(),
                                                // context starts at 8
                                                btc6->getHash());

  // apply VTB0 in VBK8
  auto vtb0containing = popminer->mineVbkBlocks(1, *vbkendorsed, {poptx0});
  auto VTB0 = popminer->createVTB(vtb0containing->getHeader(), poptx0);
  // apply VTB1 in VBK9
  auto vtb1containing = popminer->mineVbkBlocks(1, *vtb0containing, {poptx1});
  auto VTB1 = popminer->createVTB(vtb1containing->getHeader(), poptx1);
  // apply VTB2 in VBK10
  auto vtb2containing = popminer->mineVbkBlocks(1, *vtb1containing, {poptx2});
  auto VTB2 = popminer->createVTB(vtb2containing->getHeader(), poptx2);

  /// All set up. Check that env is correct
  {
    ASSERT_EQ(VTB0.containingBlock.getHeight(), 8);
    ASSERT_EQ(VTB0.transaction.blockOfProof.getHash(), btc6->getHash());
    ASSERT_EQ(VTB0.transaction.blockOfProofContext.size(), 5 /*1..5*/);

    ASSERT_EQ(VTB1.containingBlock.getHeight(), 9);
    ASSERT_EQ(VTB1.transaction.blockOfProof.getHash(), btc9->getHash());
    ASSERT_EQ(VTB1.transaction.blockOfProofContext.size(), 1 /*8*/);

    ASSERT_EQ(VTB2.containingBlock.getHeight(), 10);
    ASSERT_EQ(VTB2.transaction.blockOfProof.getHash(), btc11->getHash());
    ASSERT_EQ(VTB2.transaction.blockOfProofContext.size(), 4 /*7..10*/);
  }

  /// Send VTB0 (expect to fail stateful validation), then send connecting VBK
  /// blocks (expect them to connect VTB0)
  {
    // add to mempool
    auto result = mempool->submit<VTB>(VTB0, state);
    ASSERT_TRUE(result.isFailedStateful());
    state.reset();
    auto vbkContext0 =
        getContext(popminer->vbk(), VTB0.containingBlock.getHash(), 8);
    for (auto& block : reverse_iterate(vbkContext0)) {
      ASSERT_TRUE(mempool->submit<VbkBlock>(block, state));
    }
    // mine VTB0 to ALT1
    mineAltBlocks(1, chain, false, false);
    auto pop0 = mempool->generatePopData();
    ASSERT_EQ(pop0.vtbs.size(), 1);
    ASSERT_EQ(pop0.vtbs.at(0), VTB0);
    ASSERT_EQ(pop0.atvs.size(), 0);
    ASSERT_EQ(pop0.context.size(), 8);
    // activate ALT1
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop0));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
    // verify last known BTC/VBK
    ASSERT_EQ(getLastKnownBtcBlock(), btc6->getHash());
    ASSERT_EQ(getLastKnownVbkBlock(), vtb0containing->getHash());

    // remove accepted data from mempool
    mempool->removeAll(pop0);
  }

  /// Send VBK context + VTB2; expect it to be statefully invalid
  {
    // add to mempool
    auto result = mempool->submit<VTB>(VTB2, state);
    ASSERT_TRUE(result.isFailedStateful());
    state.reset();
    auto vbkContext2 =
        getContext(popminer->vbk(), VTB2.containingBlock.getHash(), 2);
    for (auto& block : reverse_iterate(vbkContext2)) {
      ASSERT_TRUE(mempool->submit<VbkBlock>(block, state));
    }

    // mine VTB2 in ALT2
    mineAltBlocks(1, chain, false, false);
    auto pop2 = mempool->generatePopData();
    ASSERT_EQ(pop2.vtbs.size(), 1);
    ASSERT_EQ(pop2.vtbs.at(0), VTB2);
    ASSERT_EQ(pop2.atvs.size(), 0);
    ASSERT_EQ(pop2.context.size(), 2);

    // activate ALT2
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop2));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
    // verify last known BTC/VBK
    ASSERT_EQ(getLastKnownBtcBlock(), btc11->getHash());
    ASSERT_EQ(getLastKnownVbkBlock(), vtb2containing->getHash());

    // remove accepted data from mempool
    mempool->removeAll(pop2);
  }

  ///// If VTB1 is expected to be connected, uncomment this code
  // {
  //  /// Send VTB1, expect it to connect
  //  {
  //    // add to mempool
  //    ASSERT_TRUE(mempool->submit<VTB>(VTB1, state)) << state.toString();
  //
  //    // mine VTB1 in ALT3
  //    mineAltBlocks(1, chain, false, false);
  //    auto pop1 = mempool->getPop();
  //    ASSERT_EQ(pop1.vtbs.size(), 1);
  //    ASSERT_EQ(pop1.vtbs.at(0), VTB1);
  //    ASSERT_EQ(pop1.atvs.size(), 0);
  //    ASSERT_EQ(pop1.context.size(), 0);
  //
  //    // activate ALT3
  //    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop1));
  //    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
  //    // verify last known BTC/VBK
  //    ASSERT_EQ(getLastKnownBtcBlock(), btc11->getHash());
  //    ASSERT_EQ(getLastKnownVbkBlock(), vtb2containing.getHash());
  //
  //    // remove accepted data from mempool
  //    mempool->removeAll(pop1);
  //  }
  //
  //  // at this point mempool must be empty
  //  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());
  //  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  //  ASSERT_TRUE(mempool->getMap<ATV>().empty());
  //  ASSERT_TRUE(mempool->getInFlightMap<VbkBlock>().empty());
  //  ASSERT_TRUE(mempool->getInFlightMap<VTB>().empty());
  //  ASSERT_TRUE(mempool->getInFlightMap<ATV>().empty());
  // }
}

// in this test we have a context gap in VBK which is bigger than
// maxPopDataSize() in bytes.
TEST_F(MemPoolFixture, getPop_scenario_10) {
  const auto estimatePopDataWithVbkSize = []() {
    PopData p;
    p.context.emplace_back();
    return p.estimateSize();
  };
  // PopData with 1 VBK block is 71 bytes
  const auto popDataWith1VBK = estimatePopDataWithVbkSize();
  altparam.mMaxPopDataSize =
      (uint32_t)((altparam.getMaxVbkBlocksInAltBlock() + 1) * popDataWith1VBK);
  const auto max = altparam.getMaxPopDataSize();

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());
  popminer->mineVbkBlocks(max / popDataWith1VBK + 10);
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[5];

  for (size_t i = 0; i < 5; ++i) {
    VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
        generatePublicationData(endorsedBlock1));
    auto* block = popminer->mineVbkBlocks(1, {tx});
    ATV atv = popminer->createATV(block->getHeader(), tx);
    std::vector<VbkBlock> context;
    fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

    submitATV(atv);

    for (const auto& b : context) {
      submitVBK(b);
    }
  }

  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));

  {
    PopData v_popData = checkedGetPop();
    ASSERT_LE(v_popData.estimateSize(), max);
    ASSERT_EQ(v_popData.context.size(), altparam.getMaxVbkBlocksInAltBlock());
    ASSERT_EQ(v_popData.vtbs.size(), 0);
    ASSERT_EQ(v_popData.atvs.size(), 0);
  }
  {
    // second getPop should return same data
    PopData v_popData = checkedGetPop();
    ASSERT_LE(v_popData.estimateSize(), max);
    ASSERT_EQ(v_popData.context.size(), altparam.getMaxVbkBlocksInAltBlock());
    ASSERT_EQ(v_popData.vtbs.size(), 0);
    ASSERT_EQ(v_popData.atvs.size(), 0);
    // lets remove it
    mempool->removeAll(v_popData);
  }
  {
    // third getPop returns 0, as we removed popData but not added it to
    // blockchain, so remaining data is not connected to blockchain
    PopData v_popData = checkedGetPop();
    ASSERT_EQ(v_popData.context.size(), 0);
    ASSERT_EQ(v_popData.vtbs.size(), 0);
    ASSERT_EQ(v_popData.atvs.size(), 0);
  }
}

TEST_F(MemPoolFixture, getPop_scenario_11) {
  size_t emptyPopDataSize = PopData{}.estimateSize();

  // mempool is currently empty
  for (size_t i = emptyPopDataSize; i < 1000; i++) {
    altparam.mMaxPopDataSize = (uint32_t)i;
    ASSERT_NO_FATAL_FAILURE(mempool->generatePopData());
  }

  // mine one VBK block
  auto* block_index = popminer->mineVbkBlocks(1);

  altparam.mMaxPopDataSize =
      (uint32_t)(block_index->getHeader().estimateSize() + 1);
  ASSERT_TRUE(mempool->submit(block_index->getHeader(), state));
  auto pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 0);
  ASSERT_EQ(pop_data.atvs.size(), 0);
  ASSERT_EQ(pop_data.vtbs.size(), 0);

  // mempool has 1 VBK block
  for (size_t i = emptyPopDataSize; i < 1000; i++) {
    altparam.mMaxPopDataSize = (uint32_t)i;
    ASSERT_NO_FATAL_FAILURE(mempool->generatePopData());
  }

  altparam.mMaxPopDataSize =
      (uint32_t)(block_index->getHeader().estimateSize() + 50);
  ASSERT_TRUE(mempool->submit(block_index->getHeader(), state));
  pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 1);
  ASSERT_EQ(pop_data.atvs.size(), 0);
  ASSERT_EQ(pop_data.vtbs.size(), 0);

  auto pd = popminer->endorseAltBlock(
      generatePublicationData(alttree.getBestChain().tip()->getHeader()),
      getLastKnownVbkBlock());
  ATV& atv = pd.atvs.at(0);
  ASSERT_TRUE(mempool->submit(atv, state));

  auto vtb = popminer->endorseVbkBlock(
      popminer->vbk().getBestChain().tip()->getHeader(),
      getLastKnownBtcBlock());
  ASSERT_TRUE(mempool->submit(vtb, state));

  // mempool has 3 VBK blocks, 1 VTB and 1 ATV
  for (size_t i = emptyPopDataSize; i < 5000; i++) {
    altparam.mMaxPopDataSize = (uint32_t)i;
    ASSERT_NO_FATAL_FAILURE(pop_data = mempool->generatePopData());
  }
}

TEST_F(MemPoolFixture, getPop_scenario_12) {
  size_t vbk_amount = 65;

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(vbk_amount);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  ASSERT_EQ(vbk_amount, context.size());

  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    submitVBK(*it);
  }

  // only one block should be fully valid, other blocks are statefully invalid
  ASSERT_EQ(mempool->getMap<VbkBlock>().size(), 1);
  ASSERT_EQ(mempool->getInFlightMap<VbkBlock>().size(), vbk_amount - 1);

  auto popData = checkedGetPop();

  ASSERT_EQ(popData.context.size(), vbk_amount);
}

TEST_F(MemPoolFixture, getPop_scenario_13) {
  // mine 10 ALT blocks
  mineAltBlocks(10, chain);

  // mine 10 VBK blocks
  popminer->mineVbkBlocks(10);

  // submit corrupted atvs
  AltBlock endorsedBlock = chain[5];
  endorsedBlock.previousBlock.clear();
  std::vector<VbkTx> txs;
  for (size_t i = 0; i < 1000; i++) {
    VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
        generatePublicationData(endorsedBlock));
    txs.push_back(tx);
  };

  auto* containigBlock = popminer->mineVbkBlocks(1, txs);
  for (const auto& tx : txs) {
    ATV atv = popminer->createATV(containigBlock->getHeader(), tx);
    submitATV(atv);
  }

  // mine 5 VBK blocks
  popminer->mineVbkBlocks(10);

  // submit valid ATV
  endorsedBlock = chain[5];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block = popminer->mineVbkBlocks(1, {tx});
  ATV atv = popminer->createATV(block->getHeader(), tx);
  submitATV(atv);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    submitVBK(*it);
  }

  auto pop = checkedGetPop();
  ASSERT_EQ(pop.atvs.size(), 1);
  ASSERT_EQ(pop.atvs[0], atv);
}

TEST_F(MemPoolFixture, IsKnown) {
  ASSERT_FALSE(mempool->isKnown<VbkBlock>(VbkBlock{}.getId()));
  auto next = popminer->mineVbkBlocks(1);

  ASSERT_FALSE(mempool->isKnown<VbkBlock>(next->getHeader().getId()));
  ASSERT_TRUE(mempool->submit(next->getHeader(), state));
  // VBK block is in mempool, so known
  ASSERT_TRUE(mempool->isKnown<VbkBlock>(next->getHeader().getId()));

  auto pd = checkedGetPop();
  mineAltBlocks(1, chain, false, false);

  // activate ALT1
  ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pd));
  ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
  // VBK block is in blockchain, so known
  ASSERT_TRUE(mempool->isKnown<VbkBlock>(pd.context.at(0).getId()));
}

TEST_F(MemPoolFixture, getPop_txfeePriority) {
  // ATVs with same VBK block will be compared by ATV VBK tx fee. Higher fee -
  // more chances that ATV will be picked.
  // Issue: https://github.com/VeriBlock/alt-integration-cpp/issues/890

  const auto& tx1 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(alttree.getBestChain().tip()->getHeader()),
      Coin(500));
  const auto& tx2 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(alttree.getBestChain().tip()->getHeader()),
      Coin(1000));
  const auto& block = popminer->mineVbkBlocks(1, {tx1, tx2})->getHeader();
  auto pd1 = popminer->createPopDataEndorsingAltBlock(
      block, tx1, getLastKnownVbkBlock());
  ATV& atv1 = pd1.atvs.at(0);
  ASSERT_TRUE(mempool->submit(atv1, state));
  auto pd2 = popminer->createPopDataEndorsingAltBlock(
      block, tx2, getLastKnownVbkBlock());
  ATV& atv2 = pd2.atvs.at(0);
  ASSERT_TRUE(mempool->submit(atv2, state));

  auto pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 1);
  ASSERT_EQ(pop_data.atvs.size(), 2);
  ASSERT_EQ(pop_data.vtbs.size(), 0);

  EXPECT_EQ(pop_data.atvs[0], atv2);
  EXPECT_EQ(pop_data.atvs[1], atv1);
}

TEST_F(MemPoolFixture, getPop_endorsedPriority) {
  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[6];
  AltBlock endorsedBlock2 = chain[5];

  const auto& tx1 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock1), Coin(1000));
  const auto& tx2 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock2), Coin(1000));
  const auto& block = popminer->mineVbkBlocks(1, {tx1, tx2})->getHeader();
  auto pd1 = popminer->createPopDataEndorsingAltBlock(
      block, tx1, getLastKnownVbkBlock());
  ATV& atv1 = pd1.atvs.at(0);
  ASSERT_TRUE(mempool->submit(atv1, state));
  auto pd2 = popminer->createPopDataEndorsingAltBlock(
      block, tx2, getLastKnownVbkBlock());
  ATV& atv2 = pd2.atvs.at(0);
  ASSERT_TRUE(mempool->submit(atv2, state));

  auto pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 1);
  ASSERT_EQ(pop_data.atvs.size(), 2);
  ASSERT_EQ(pop_data.vtbs.size(), 0);

  EXPECT_EQ(pop_data.atvs[0], atv2);
  EXPECT_EQ(pop_data.atvs[1], atv1);
}

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
  auto* vbkTip = popminer->mineVbkBlocks(20);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);

  // generate two VTBs
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  // generate two ATVs
  const auto& tx1 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock1), Coin(1000));
  const auto& tx2 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock2), Coin(1000));
  const auto& block = popminer->mineVbkBlocks(1, {tx1, tx2})->getHeader();
  auto pd1 = popminer->createPopDataEndorsingAltBlock(
      block, tx1, getLastKnownVbkBlock());
  ATV& atv1 = pd1.atvs.at(0);
  auto pd2 = popminer->createPopDataEndorsingAltBlock(
      block, tx2, getLastKnownVbkBlock());
  ATV& atv2 = pd2.atvs.at(0);

  // generate VBK fork block
  auto* fork_block = popminer->mineVbkBlocks(1, *vbkTip);

  // mine vbk blocks more than "old block" window
  vbkTip = popminer->mineVbkBlocks(vbkparam.getOldBlocksWindow() + 5);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    ASSERT_TRUE(mempool->submit(*it, state));
  }

  state.reset();

  while (alttree.vbk().getBestChain().tip()->getHeight() !=
         vbkTip->getHeight()) {
    auto pop_data = mempool->generatePopData();
    mineAltBlocks(1, chain, false, false);
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop_data));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
  }

  ASSERT_FALSE(mempool->submit(atv1, state));
  ASSERT_FALSE(mempool->submit(atv2, state));
  ASSERT_FALSE(mempool->submit(fork_block->getHeader(), state));

  ASSERT_TRUE(mempool->submit(vtb1, state));
  ASSERT_TRUE(mempool->submit(vtb2, state));
}

TEST_F(MemPoolFixture, getPop_payloads_order1) {
  // Payloads with earlier VBK block will be earlier. VBK = use itself,
  // ATV=block of proof, VTB=containing.
  // Issue: https://github.com/VeriBlock/alt-integration-cpp/issues/890

  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[6];
  AltBlock endorsedBlock2 = chain[5];

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  ASSERT_GE(vbkTip->getHeight(), 11);

  const auto& tx1 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock1), Coin(1000));
  const auto& tx2 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
      generatePublicationData(endorsedBlock2), Coin(1000));
  const auto& block = popminer->mineVbkBlocks(1, {tx1, tx2})->getHeader();
  auto pd1 = popminer->createPopDataEndorsingAltBlock(
      block, tx1, getLastKnownVbkBlock());
  ATV& atv1 = pd1.atvs.at(0);
  auto pd2 = popminer->createPopDataEndorsingAltBlock(
      block, tx2, getLastKnownVbkBlock());
  ATV& atv2 = pd2.atvs.at(0);

  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    ASSERT_TRUE(mempool->submit(*it, state));
  }

  ASSERT_TRUE(mempool->submit(atv1, state));
  ASSERT_TRUE(mempool->submit(atv2, state));

  ASSERT_TRUE(mempool->submit(vtb1, state));
  ASSERT_TRUE(mempool->submit(vtb2, state));

  auto pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 67);
  ASSERT_EQ(pop_data.atvs.size(), 2);
  ASSERT_EQ(pop_data.vtbs.size(), 2);

  // check VBK blocks order
  for (size_t i = 1; i < pop_data.context.size(); i++) {
    ASSERT_LE(pop_data.context[i - 1].getHeight(),
              pop_data.context[i].getHeight());
  }

  for (size_t i = 1; i < pop_data.atvs.size(); i++) {
    ASSERT_LE(pop_data.atvs[i - 1].blockOfProof.getHeight(),
              pop_data.atvs[i].blockOfProof.getHeight());
  }

  for (size_t i = 1; i < pop_data.vtbs.size(); i++) {
    ASSERT_LE(pop_data.vtbs[i - 1].containingBlock.getHeight(),
              pop_data.vtbs[i].containingBlock.getHeight());
  }
}

// TEST_F(MemPoolFixture, getPop_payloads_order2) {
//   // Payloads with VBK block on active chain will be earlier.
//   // Issue: https://github.com/VeriBlock/alt-integration-cpp/issues/890

//   // mine 10 blocks
//   mineAltBlocks(10, chain);
//   AltBlock endorsedBlock1 = chain[6];
//   AltBlock endorsedBlock2 = chain[5];

//   // mine 30 VBK blocks
//   auto* vbkForkPoint = popminer->mineVbkBlocks(30);

//   auto* vbkChainA = popminer->mineVbkBlocks(15, *vbkForkPoint);
//   auto* vbkChainB = popminer->mineVbkBlocks(35, *vbkForkPoint);

//   ASSERT_EQ(popminer->vbk().getBestChain().tip(), vbkChainB);

//   // endorse VBK blocks
//   ASSERT_GE(vbkChainA->getHeight(), 11);
//   ASSERT_GE(vbkChainB->getHeight(), 11);

//   const auto& tx1 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
//       generatePublicationData(endorsedBlock1), Coin(1000));
//   const auto& tx2 = popminer->createVbkTxEndorsingAltBlockWithSourceAmount(
//       generatePublicationData(endorsedBlock2), Coin(1000));
//   vbkChainA = popminer->mineVbkBlocks(1, *vbkChainA, {tx1});
//   auto pd1 = popminer->createPopDataEndorsingAltBlock(
//       vbkChainA->getHeader(), tx1, getLastKnownVbkBlock());
//   ATV& atv1 = pd1.atvs.at(0);
//   vbkChainB = popminer->mineVbkBlocks(1, *vbkChainB, {tx2});
//   auto pd2 = popminer->createPopDataEndorsingAltBlock(
//       vbkChainB->getHeader(), tx2, getLastKnownVbkBlock());
//   ATV& atv2 = pd2.atvs.at(0);

//   const auto* endorsedVbkBlock1 =
//       vbkChainA->getAncestor(vbkChainA->getHeight() - 10);
//   const auto* endorsedVbkBlock2 =
//       vbkChainB->getAncestor(vbkChainB->getHeight() - 11);
//   auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
//   popminer->mineBtcBlocks(100);
//   auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

//   vbkChainA = popminer->mineVbkBlocks(1, *vbkChainA, {vbkPopTx1});
//   vbkChainB = popminer->mineVbkBlocks(1, *vbkChainB, {vbkPopTx2});

//   auto vtb1 = popminer->createVTB(vbkChainA->getHeader(), vbkPopTx1);
//   auto vtb2 = popminer->createVTB(vbkChainB->getHeader(), vbkPopTx2);

//   std::vector<VbkBlock> context;
//   fillVbkContext(context,
//                  GetRegTestVbkBlock().getHash(),
//                  vbkChainA->getHash(),
//                  popminer->vbk());
//   for (auto it = context.rbegin(); it != context.rend(); ++it) {
//     ASSERT_TRUE(mempool->submit(*it, state));
//   }
//   context.clear();
//   fillVbkContext(context,
//                  GetRegTestVbkBlock().getHash(),
//                  vbkChainB->getHash(),
//                  popminer->vbk());
//   for (auto it = context.rbegin(); it != context.rend(); ++it) {
//     ASSERT_TRUE(mempool->submit(*it, state));
//   }

//   ASSERT_TRUE(mempool->submit(atv1, state));
//   ASSERT_TRUE(mempool->submit(atv2, state));

//   ASSERT_TRUE(mempool->submit(vtb1, state));
//   ASSERT_TRUE(mempool->submit(vtb2, state));

//   auto pop_data = mempool->generatePopData();
//   ASSERT_EQ(pop_data.context.size(), 84);
//   ASSERT_EQ(pop_data.atvs.size(), 2);
//   ASSERT_EQ(pop_data.vtbs.size(), 2);

//   ASSERT_EQ(popminer->vbk().getBestChain().tip(), vbkChainB);

//   // check payloads order
//   ASSERT_EQ(pop_data.atvs[0].getId(), atv2.getId());
//   ASSERT_EQ(pop_data.atvs[1].getId(), atv1.getId());

//   ASSERT_EQ(pop_data.vtbs[0].getId(), vtb2.getId());
//   ASSERT_EQ(pop_data.vtbs[1].getId(), vtb1.getId());
// }

TEST_F(MemPoolFixture, vbkblocks_cleanup_bug) {
  // For some reason VBK statefully duplicate blocks remain in mempool (should
  // be removed). Issue:
  // https://github.com/VeriBlock/alt-integration-cpp/issues/893

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  std::vector<VbkBlock> context;
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());
  for (auto it = context.rbegin(); it != context.rend(); ++it) {
    ASSERT_TRUE(mempool->submit(*it, state));
  }

  auto pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 65);

  state.reset();
  mineAltBlocks(1, chain, false, false);
  ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop_data));
  ASSERT_TRUE(SetState(alttree, chain.back().getHash()));

  pop_data = mempool->generatePopData();
  ASSERT_EQ(pop_data.context.size(), 0);
  ASSERT_EQ(mempool->getMap<VbkBlock>().size() +
                mempool->getInFlightMap<VbkBlock>().size(),
            0);
}
