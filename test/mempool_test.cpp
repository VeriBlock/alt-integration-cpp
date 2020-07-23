// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mempool.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "util/pop_test_fixture.hpp"
#include "util/test_utils.hpp"
#include "veriblock/hashutil.hpp"

using namespace altintegration;

struct MemPoolFixture : public PopTestFixture, public ::testing::Test {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  MemPoolFixture() { GetLogger().level = altintegration::LogLevel::info; }

  void applyInNextBlock(const PopData& pop) {
    auto containingBlock = generateNextBlock(*chain.rbegin());
    chain.push_back(containingBlock);
    ASSERT_TRUE(alttree.acceptBlock(containingBlock, state));
    ASSERT_TRUE(alttree.addPayloads(containingBlock.getHash(), pop, state))
        << state.toString();
    ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
    ASSERT_TRUE(state.IsValid());
  }

  PopData checkedGetPop() {
    auto before = alttree.toPrettyString();
    auto ret = mempool->getPop(alttree);
    auto after = alttree.toPrettyString();
    EXPECT_EQ(before, after);
    return ret;
  }

  void removeLastAltBlock() {
    alttree.removeSubtree(chain.rbegin()->getHash());
    chain.pop_back();
    ValidationState dummy;
    alttree.setState(chain.rbegin()->getHash(), dummy);
  }
};

TEST_F(MemPoolFixture, removePayloads_test1) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  fillVbkContext(
      vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(
      vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(0), alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(1), alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // do the same to show that from mempool do not remove payloads
  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // remove from mempool
  mempool->removePayloads(popData, alttree);

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));

  ASSERT_TRUE(mempool->getMap<ATV>().empty());
  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());

  popData = checkedGetPop();
  EXPECT_EQ(popData.context.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 0);
  EXPECT_EQ(popData.atvs.size(), 0);
}

TEST_F(MemPoolFixture, removePayloads_test2) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  fillVbkContext(
      vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(
      vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(0), alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(1), alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);

  // modify popData to not remove all payloads
  popData.atvs.clear();

  mempool->removePayloads(popData, alttree);

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));

  ASSERT_FALSE(mempool->getMap<ATV>().empty());
  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  ASSERT_FALSE(mempool->getMap<VbkBlock>().empty());
  ASSERT_EQ(mempool->getMap<VbkBlock>().size(), 1);
  ASSERT_EQ(mempool->getMap<ATV>().size(), 1);
  ASSERT_EQ(mempool->getMap<VbkBlock>().begin()->second->getHash(),
            mempool->getMap<ATV>().begin()->second->blockOfProof.getHash());
}

TEST_F(MemPoolFixture, removePayloads_test3) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  fillVbkContext(
      vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(
      vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(0), alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtbs.at(1), alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_EQ(popData.atvs.at(0), atv);
  EXPECT_FALSE(popData.context.empty());

  size_t prev_size = popData.context.size();
  // remove from popData payloads containing vbk blocks
  for (auto it = popData.context.begin(); it != popData.context.end();) {
    if (it->getHash() == popData.atvs[0].blockOfProof.getHash() ||
        it->getHash() == popData.vtbs[0].containingBlock.getHash() ||
        it->getHash() == popData.vtbs[1].containingBlock.getHash()) {
      it = popData.context.erase(it);
    }
    ++it;
  }

  EXPECT_EQ(popData.context.size(), prev_size - 3);

  mempool->removePayloads(popData, alttree);

  ASSERT_FALSE(mempool->getMap<ATV>().empty());
  ASSERT_TRUE(mempool->getMap<VTB>().empty());
  // ASSERT_TRUE(mempool->getMap<VbkBlock>().empty());
}

TEST_F(MemPoolFixture, removed_payloads_cache_test) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  fillVbkContext(
      vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(
      vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit(atv, alttree, state));
  for (const auto& vtb : vtbs) {
    EXPECT_TRUE(mempool->submit(vtb, alttree, state));
  }

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData popData = mempool->getPop(alttree);

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_FALSE(popData.context.empty());
  EXPECT_EQ(popData.atvs.at(0), atv);

  applyInNextBlock(popData);
  mempool->removePayloads(popData, alttree);

  popData = mempool->getPop(alttree);

  EXPECT_TRUE(popData.vtbs.empty());
  EXPECT_TRUE(popData.atvs.empty());
  EXPECT_TRUE(popData.context.empty());

  // insert the same payloads into the mempool
  EXPECT_FALSE(mempool->submit(atv, alttree, state));
  for (const auto& vtb : vtbs) {
    EXPECT_FALSE(mempool->submit(vtb, alttree, state));
  }

  popData = mempool->getPop(alttree);
  EXPECT_TRUE(popData.vtbs.empty());
  EXPECT_TRUE(popData.atvs.empty());
  EXPECT_TRUE(popData.context.empty());
}

TEST_F(MemPoolFixture, submit_deprecated_payloads) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  popminer->mineVbkBlocks(
      popminer->vbk().getParams().getEndorsementSettlementInterval());

  std::vector<VbkPopTx> txes;
  txes.push_back(generatePopTx(endorsedVbkBlock1->getHeader()));
  popminer->mineBtcBlocks(100);
  txes.push_back(generatePopTx(endorsedVbkBlock2->getHeader()));

  // remove invalid vbk_poptxes
  popminer->vbkmempool.clear();

  // generate vtbs with the deprecated endorsements
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>(txes);
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingBlock = vbk_miner.createNextBlock(
      *popminer->vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());
  EXPECT_TRUE(popminer->vbk().acceptBlock(containingBlock, state));

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
                   vtb.merklePath.layers =
                       mtree.getMerklePathLayers(hashes[index]);
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

  mineAltBlocks(alttree.getParams().getEndorsementSettlementInterval(), chain);

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  // insert the same payloads into the mempool
  EXPECT_FALSE(mempool->submit(atv, alttree, state));
  for (const auto& vtb : vtbs) {
    EXPECT_TRUE(checkVTB(
        vtb, state, popminer->getVbkParams(), popminer->getBtcParams()));
    EXPECT_FALSE(mempool->submit(vtb, alttree, state));
  }
}

TEST_F(MemPoolFixture, getPop_scenario_1) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs[0]).id,
            VbkEndorsement::fromContainer(vtbs[1]).id);
  ASSERT_NE(vtbs[0].getId(), vtbs[1].getId());

  fillVbkContext(vtbs[0].context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs[0].containingBlock.getHash(),
                 popminer->vbk());
  fillVbkContext(vtbs[1].context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs[1].containingBlock.getHash(),
                 popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  ASSERT_TRUE(mempool->submit<ATV>(atv, alttree, state)) << state.toString();
  ASSERT_TRUE(mempool->submit<VTB>(vtbs[0], alttree, state))
      << state.toString();
  ASSERT_TRUE(mempool->submit<VTB>(vtbs[1], alttree, state))
      << state.toString();

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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
  generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads.at(containingVbkBlock1->getHash()).size(), 1);
  VTB vtb1 = popminer->vbkPayloads.at(containingVbkBlock1->getHash()).at(0);
  fillVbkContext(vtb1.context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtb1.containingBlock.getHash(),
                 popminer->vbk());

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVbkContext(vtb2, containingVbkBlock1->getHash(), popminer->vbk());

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  vbkTip = popminer->vbk().getBestChain().tip();

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtb2, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 2);

  applyInNextBlock(v_popData);
}

TEST_F(MemPoolFixture, getPop_scenario_3) {
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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
  ATV atv = popminer->generateATV(
      tx, popminer->vbk().getParams().getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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

  generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer->generateATV(tx1, containingVbkBlock1->getHash(), state);

  vbkTip = popminer->vbk().getBestChain().tip();

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVbkContext(vtb2, vbkTip->getHash(), popminer->vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer->generateATV(tx2, containingVbkBlock2->getHash(), state);

  {
    EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
    EXPECT_TRUE(mempool->submit<ATV>(atv2, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb2, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));

    ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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
    EXPECT_TRUE(mempool->submit<ATV>(atv2, alttree, state));
    EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb2, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));

    ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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
    // No ATV2
    EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));
    EXPECT_TRUE(mempool->submit<VTB>(vtb2, alttree, state));

    ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

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
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer->vbk().getParams(), popminer->btc().getParams()));

  EXPECT_NE(vtb1.containingBlock, vtb2.containingBlock);
  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  EXPECT_EQ(E1.payoutInfo, E2.payoutInfo);
  EXPECT_EQ(E1.blockOfProof, E2.blockOfProof);
  EXPECT_EQ(E1.endorsedHash, E2.endorsedHash);
  EXPECT_NE(E1.containingHash, E2.containingHash);
  EXPECT_EQ(E1.id, E2.id);

  fillVbkContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtb2, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.context.size(), 122);
  EXPECT_EQ(v_popData.vtbs.size(), 1);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.vtbs.at(0), vtb1);

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
  ATV atv1 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData v_popData = checkedGetPop();
  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.atvs.at(0), atv1);
  applyInNextBlock(v_popData);

  mempool->removePayloads(v_popData, alttree);
  ASSERT_FALSE(mempool->submit(atv1, alttree, state)) << state.toString();
  ASSERT_EQ(state.GetPath(), "pop-mempool-submit-atv-duplicate");
}

TEST_F(MemPoolFixture, getPop_scenario_8) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);

  auto vbkPopTx = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit<ATV>(atv1, alttree, state));
  EXPECT_TRUE(mempool->submit<VTB>(vtb1, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 1);
  EXPECT_EQ(v_popData.vtbs.at(0), vtb1);

  applyInNextBlock(v_popData);

  // remove payloads from the mempool
  mempool->removePayloads(v_popData, alttree);

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
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer->vbk().getParams(), popminer->btc().getParams()));

  EXPECT_NE(vtb1.containingBlock, vtb2.containingBlock);
  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  EXPECT_EQ(E1.payoutInfo, E2.payoutInfo);
  EXPECT_EQ(E1.blockOfProof, E2.blockOfProof);
  EXPECT_EQ(E1.endorsedHash, E2.endorsedHash);
  EXPECT_NE(E1.containingHash, E2.containingHash);
  EXPECT_EQ(E1.id, E2.id);

  fillVbkContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  ATV atv2 =
      popminer->generateATV(tx2, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit(atv2, alttree, state));
  EXPECT_FALSE(mempool->submit(vtb2, alttree, state)) << state.toString();

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.context.size(), 56);
}
// This test scenrio tests filter payloads duplicates in the same altblock chain
TEST_F(MemPoolFixture, getPop_scenario_9) {
  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit(atv1, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  PopData v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.context.size(), 66);
  EXPECT_EQ(v_popData.vtbs.size(), 0);
  EXPECT_EQ(v_popData.atvs.size(), 1);
  EXPECT_EQ(v_popData.atvs.at(0), atv1);

  applyInNextBlock(v_popData);

  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv2 =
      popminer->generateATV(tx2, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool->submit(atv2, alttree, state));

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
  v_popData = checkedGetPop();

  EXPECT_EQ(v_popData.atvs.at(0), atv2);
  EXPECT_EQ(v_popData.context.size(), 1);
  EXPECT_EQ(v_popData.vtbs.size(), 0);

  applyInNextBlock(v_popData);
}

// in this test we have a context gap in VBK which is bigger than
// maxPopDataSize() in bytes.
// we expect that first getPop returns PopData which contains around 15k VBK
// blocks (maxPopDataSize / 71 = 14768), 0 VTBs, and 0 ATVs
TEST_F(MemPoolFixture, getPop_scenario_10) {
  const auto estimatePopDataWithVbkSize = []() {
    PopData p;
    p.context.emplace_back();
    return p.estimateSize();
  };
  // PopData with 1 VBK block is 71 bytes
  const auto popDataWith1VBK = estimatePopDataWithVbkSize();
  const auto max = alttree.getParams().getMaxPopDataSize();

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());
  popminer->mineVbkBlocks(max / popDataWith1VBK + 10);
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[5];

  for (size_t i = 0; i < 5; ++i) {
    VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
        generatePublicationData(endorsedBlock1));
    ATV atv =
        popminer->generateATV(tx, vbkparam.getGenesisBlock().getHash(), state);

    EXPECT_TRUE(mempool->submit<ATV>(atv, alttree, state));
  }

  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));

  {
    PopData v_popData = checkedGetPop();
    ASSERT_LE(v_popData.estimateSize(), max);
    ASSERT_EQ(v_popData.context.size(), max / popDataWith1VBK);
    ASSERT_EQ(v_popData.vtbs.size(), 0);
    ASSERT_EQ(v_popData.atvs.size(), 0);
  }
  {
    // second getPop should return same data
    PopData v_popData = checkedGetPop();
    ASSERT_LE(v_popData.estimateSize(), max);
    ASSERT_EQ(v_popData.context.size(), max / popDataWith1VBK);
    ASSERT_EQ(v_popData.vtbs.size(), 0);
    ASSERT_EQ(v_popData.atvs.size(), 0);
    // lets remove it
    mempool->removePayloads(v_popData, alttree);
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

// This test scenrio tests the possible context gap in case that all payloads
// contain in the same PopData which bigger than maxPopDataSize
TEST_F(MemPoolFixture, getPop_scenario_11) {
  auto* vbkTip = popminer->mineVbkBlocks(65);

  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  size_t vtbs_amount = 100;
  for (size_t i = 0; i < vtbs_amount; ++i) {
    popminer->mineBtcBlocks(100);
    generatePopTx(endorsedVbkBlock1->getHeader());
  }

  ASSERT_EQ(popminer->vbkmempool.size(), vtbs_amount);

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  fillVbkContext(vtbs[0].context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs[0].containingBlock.getHash(),
                 popminer->vbk());

  ASSERT_EQ(vtbs.size(), vtbs_amount);

  for (const auto& vtb : vtbs) {
    ASSERT_TRUE(mempool->submit<VTB>(vtb, alttree, state)) << state.toString();
  }

  ASSERT_EQ(mempool->getMap<VTB>().size(), vtbs_amount);

  PopData pop = checkedGetPop();

  EXPECT_TRUE(pop.vtbs.size() < vtbs_amount);
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
  EXPECT_TRUE(vbk_blocks.rbegin()->getHash() ==
              popminer->vbk().getBestChain().tip()->getHash());

  for (size_t i = 0; i < vbk_blocks.size(); ++i) {
    EXPECT_TRUE(mempool->submit<VbkBlock>(vbk_blocks[i], alttree, state));
    EXPECT_TRUE(state.IsValid());
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

  generatePopTx(endorsedVbkBlock1->getHeader());
  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock->getHash()].size(), 2);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock->getHash()][0];
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock->getHash()][1];
  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  EXPECT_EQ(vtb1.containingBlock, vtb2.containingBlock);

  mempool->submit<VTB>(vtb1, alttree, state);

  PopData popData = checkedGetPop();

  EXPECT_EQ(popData.atvs.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 1);
  EXPECT_TRUE(!popData.context.empty());

  applyInNextBlock(popData);
  mempool->removePayloads(popData, alttree);

  mempool->submit<VTB>(vtb2, alttree, state);

  popData = checkedGetPop();

  EXPECT_EQ(popData.atvs.size(), 0);
  EXPECT_EQ(popData.vtbs.size(), 1);
  EXPECT_TRUE(popData.context.empty());

  applyInNextBlock(popData);
}