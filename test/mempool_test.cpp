// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <vector>
#include <veriblock/alt-util.hpp>

#include "util/pop_test_fixture.hpp"
#include "util/test_utils.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/mempool.hpp"

using namespace altintegration;

struct MemPoolFixture : public PopTestFixture, public ::testing::Test {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  MemPoolFixture() { GetLogger().level = altintegration::LogLevel::info; }

  void applyInNextBlock(const PopData& pop) {
    auto containingBlock = generateNextBlock(chain.back());
    chain.push_back(containingBlock);
    ASSERT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
    ASSERT_TRUE(state.IsValid()) << state.toString();
    ASSERT_TRUE(AddPayloads(containingBlock.getHash(), pop))
        << state.toString();
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
    ASSERT_TRUE(state.IsValid());
    validateAlttreeIndexState(alttree, containingBlock, pop);
  }

  PopData checkedGetPop() {
    auto before = alttree.toPrettyString();
    auto ret = mempool->getPop();
    auto after = alttree.toPrettyString();
    EXPECT_EQ(before, after);
    return ret;
  }

  void removeLastAltBlock() {
    alttree.removeSubtree(chain.back().getHash());
    chain.pop_back();
    ValidationState dummy;
    alttree.setState(chain.back().getHash(), dummy);
  }

  void submitATV(const ATV& atv) {
    mempool->submit(atv, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "atv-stateful");
      state.reset();
    }
  }

  void submitVTB(const VTB& vtb) {
    mempool->submit(vtb, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "vtb-stateful");
      state.reset();
    }
  }

  void submitVBK(const VbkBlock& vbk) {
    mempool->submit(vbk, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "vbk-stateful");
      state.reset();
    }
  }
};

TEST_F(MemPoolFixture, removeAll_test1) {
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

  // mine 10 blocks
  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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
  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(15);

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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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
  generatePopTx(popminer->vbk().getBestChain().tip()->getHeader());
  vbkTip = popminer->mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  EXPECT_EQ(vtbs.size(), 1);

  context.clear();
  fillVbkContext(context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  submitVTB(vtbs[0]);
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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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
  PopData popData = mempool->getPop();

  EXPECT_EQ(popData.vtbs.size(), 2);
  EXPECT_EQ(popData.atvs.size(), 1);
  EXPECT_FALSE(popData.context.empty());
  EXPECT_EQ(popData.atvs.at(0), atv);

  applyInNextBlock(popData);
  mempool->removeAll(popData);

  popData = mempool->getPop();

  EXPECT_TRUE(popData.vtbs.empty());
  EXPECT_TRUE(popData.atvs.empty());
  EXPECT_TRUE(popData.context.empty());

  // insert the same payloads into the mempool
  submitATV(atv);
  EXPECT_EQ(mempool->getMap<ATV>().size(), 0);
  for (const auto& vtb : vtbs) {
    submitVTB(vtb);
    EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
  }

  popData = mempool->getPop();
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

  mineAltBlocks(alttree.getParams().getPayoutParams().getPopPayoutDelay(),
                chain);

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

  // insert the same payloads into the mempool
  submitATV(atv);
  EXPECT_EQ(mempool->getMap<ATV>().size(), 0);
  for (const auto& vtb : vtbs) {
    EXPECT_TRUE(checkVTB(vtb, state, popminer->getBtcParams()));
    submitVTB(vtb);
    EXPECT_EQ(mempool->getMap<VTB>().size(), 0);
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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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
  generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads.at(containingVbkBlock1->getHash()).size(), 1);
  VTB vtb1 = popminer->vbkPayloads.at(containingVbkBlock1->getHash()).at(0);

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  vbkTip = popminer->vbk().getBestChain().tip();

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv = popminer->applyATV(tx, state);

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
  ATV atv = popminer->applyATV(tx, state);

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
  ATV atv = popminer->applyATV(tx, state);

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

  generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer->applyATV(tx1, state);

  vbkTip = popminer->vbk().getBestChain().tip();

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer->applyATV(tx2, state);

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

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
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
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(vtb2, state, popminer->btc().getParams()));

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
  ATV atv1 = popminer->applyATV(tx1, state);

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
  ATV atv1 = popminer->applyATV(tx1, state);

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

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer->applyATV(tx1, state);

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
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(vtb2, state, popminer->btc().getParams()));

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
  ATV atv2 = popminer->applyATV(tx2, state);
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
  ATV atv1 = popminer->applyATV(tx1, state);
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
  ATV atv2 = popminer->applyATV(tx2, state);
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
  for (size_t i = 0; i < vtbs_amount; ++i) {
    popminer->mineBtcBlocks(10);
    generatePopTx(endorsedVbkBlock1->getHeader());
  }

  ASSERT_EQ(popminer->vbkmempool.size(), vtbs_amount);

  vbkTip = popminer->mineVbkBlocks(1);

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

  generatePopTx(endorsedVbkBlock1->getHeader());
  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock = popminer->mineVbkBlocks(1);
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

  generatePopTx(endorsedVbkBlock1->getHeader());
  generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock = popminer->mineVbkBlocks(1);
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
  popminer->btcmempool.push_back(btctx0);
  auto btc6 = popminer->mineBtcBlocks(1);
  ASSERT_EQ(btc6->getHeight(), 6);
  auto btc7 = popminer->mineBtcBlocks(1);
  // BTC8
  popminer->mineBtcBlocks(1);
  // mine VTB1 block of proof
  popminer->btcmempool.push_back(btctx1);
  auto btc9 = popminer->mineBtcBlocks(1);
  ASSERT_EQ(btc9->getHeight(), 9);
  // BTC10
  popminer->mineBtcBlocks(1);
  // mine VTB2 block of proof
  popminer->btcmempool.push_back(btctx2);
  auto btc11 = popminer->mineBtcBlocks(1);
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
  auto vtb0containing = popminer->applyVTB(
      vbkendorsed->getHash(), popminer->vbk(), poptx0, state);
  auto& VTB0 = popminer->vbkPayloads.at(vtb0containing.getHash()).at(0);
  // apply VTB1 in VBK9
  auto vtb1containing = popminer->applyVTB(
      vtb0containing.getHash(), popminer->vbk(), poptx1, state);
  auto& VTB1 = popminer->vbkPayloads.at(vtb1containing.getHash()).at(0);
  // apply VTB2 in VBK10
  auto vtb2containing = popminer->applyVTB(
      vtb1containing.getHash(), popminer->vbk(), poptx2, state);
  auto& VTB2 = popminer->vbkPayloads.at(vtb2containing.getHash()).at(0);

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
    auto pop0 = mempool->getPop();
    ASSERT_EQ(pop0.vtbs.size(), 1);
    ASSERT_EQ(pop0.vtbs.at(0), VTB0);
    ASSERT_EQ(pop0.atvs.size(), 0);
    ASSERT_EQ(pop0.context.size(), 8);
    // activate ALT1
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop0));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
    // verify last known BTC/VBK
    ASSERT_EQ(getLastKnownBtcBlock(), btc6->getHash());
    ASSERT_EQ(getLastKnownVbkBlock(), vtb0containing.getHash());

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
    auto pop2 = mempool->getPop();
    ASSERT_EQ(pop2.vtbs.size(), 1);
    ASSERT_EQ(pop2.vtbs.at(0), VTB2);
    ASSERT_EQ(pop2.atvs.size(), 0);
    ASSERT_EQ(pop2.context.size(), 2);

    // activate ALT2
    ASSERT_TRUE(AddPayloads(alttree, chain.back().getHash(), pop2));
    ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
    // verify last known BTC/VBK
    ASSERT_EQ(getLastKnownBtcBlock(), btc11->getHash());
    ASSERT_EQ(getLastKnownVbkBlock(), vtb2containing.getHash());

    // remove accepted data from mempool
    mempool->removeAll(pop2);
  }

  // At this moment, VTB1 is not expected to be connected, because BTC block 8
  // can not be connected to previous block 6.
  // add to mempool
  ASSERT_TRUE(mempool->submit<VTB>(VTB1, state)) << state.toString();
  // mine VTB1 in ALT3
  mineAltBlocks(1, chain, false, false);
  auto pop1 = mempool->getPop();
  ASSERT_EQ(pop1.vtbs.size(), 0);
  ASSERT_EQ(pop1.atvs.size(), 0);
  ASSERT_EQ(pop1.context.size(), 0);

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
      (altparam.getMaxVbkBlocksInAltBlock() + 1) * popDataWith1VBK;
  const auto max = altparam.getMaxPopDataSize();

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());
  popminer->mineVbkBlocks(max / popDataWith1VBK + 10);
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock1 = chain[5];

  for (size_t i = 0; i < 5; ++i) {
    VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
        generatePublicationData(endorsedBlock1));
    ATV atv = popminer->applyATV(tx, state);
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
