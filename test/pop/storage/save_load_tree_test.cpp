// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct SaveLoadTreeTest : public PopTestFixture, public testing::Test {
  SaveLoadTreeTest() {
    alttree2.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), state);
    alttree2.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), state);
    bool ok = alttree2.bootstrap(state);
    VBK_ASSERT_MSG(ok, "Can not bootstrap ALT tree: %s", state.toString());

    chain.push_back(altparam.getBootstrapBlock());
    createEndorsedAltChain(20, 3);
  }

  std::vector<AltBlock> chain;

  AltBlockTree alttree2 = AltBlockTree(
      altparam, vbkparam, btcparam, payloadsProvider, blockProvider);

  auto assertTreesEqual() {
    assertTreesHaveNoOrphans(alttree);
    assertTreesHaveNoOrphans(alttree2);
    ASSERT_TRUE(cmp(alttree, alttree2));
    auto tip = alttree.getBestChain().tip();
    auto to1 = alttree.getBestChain().first()->getHash();
    EXPECT_TRUE(alttree.setState(to1, state));
    EXPECT_TRUE(alttree2.setState(to1, state));
    ASSERT_TRUE(cmp(alttree, alttree2));
    auto to2 = tip->getHash();
    EXPECT_TRUE(alttree.setState(to2, state));
    EXPECT_TRUE(alttree2.setState(to2, state));
    ASSERT_TRUE(cmp(alttree, alttree2));
  }
};

// alttree does not contain any invalid blocks
TEST_F(SaveLoadTreeTest, ValidTree) {
  save(alttree);
  ASSERT_TRUE(load(alttree2)) << state.toString();
  assertTreesEqual();
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_NoUnconnectedBlockCheck) {
  VBK_LOG_DEBUG("mine 20 alt blocks");

  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  VBK_LOG_DEBUG("add alt payloads");

  PopData popData = generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  VBK_LOG_DEBUG(
      "mine a buffer block that will prevent the next block from being "
      "connected on setPayloads()");

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));

  VBK_LOG_DEBUG("mine a block that contains statefully duplicate payloads");

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  alttree.acceptBlock(containingBlock.getHash(), popData);

  save();

  EXPECT_TRUE(load());
  EXPECT_TRUE(state.IsValid());
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData popData = generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  alttree.acceptBlock(containingBlock.getHash(), {});

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  containingIndex->template insertPayloadIds<VbkBlock>(
      map_get_id(popData.context));

  for (const auto& b : popData.context) {
    alttree.getPayloadsIndex().addAltPayloadIndex(containingBlock.getHash(),
                                                  b.getId().asVector());
  }

  save(alttree);

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "failed-to-load-alt-tree+load-tree+VBK-duplicate");
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test2) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData popData = generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());

  containingIndex->template insertPayloadIds<VbkBlock>(
      map_get_id(popData.context));

  for (const auto& b : popData.context) {
    alttree.getPayloadsIndex().addAltPayloadIndex(containingBlock.getHash(),
                                                  b.getId().asVector());
  }

  save(alttree);

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "failed-to-load-alt-tree+load-tree+VBK-duplicate");
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test3) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData popData = generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  alttree.acceptBlock(containingBlock.getHash(), {});

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  containingIndex->template insertPayloadIds<ATV>(map_get_id(popData.atvs));

  // add duplicates
  // add duplicate atv
  alttree.getPayloadsIndex().addAltPayloadIndex(
      containingBlock.getHash(), popData.atvs[0].getId().asVector());

  save(alttree);

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "failed-to-load-alt-tree+load-tree+ATV-duplicate");
}

TEST_F(SaveLoadTreeTest, ReloadWithDuplicatesVbk_test1) {
  PopData popData;

  auto vbkPopTx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      popminer->vbkTip()->getHeader(), popminer->btcTip()->getHash());

  // both VTBs should be contained in the same block
  BlockIndex<VbkBlock>* containingVbkBlock =
      popminer->mineVbkBlocks(1, {vbkPopTx1});

  auto vtb1 = popminer->createVTB(containingVbkBlock->getHeader(), vbkPopTx1);
  popData.vtbs = {vtb1};
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock->getHash(),
                 popminer->vbk());
  payloadsProvider.writePayloads(popData);

  // manually add the VBK context to alttree
  for (auto& block : popData.context) {
    ASSERT_TRUE(alttree.vbk().acceptBlockHeader(block, state))
        << state.toString();
  }

  ASSERT_TRUE(alttree.vbk().addPayloads(
      containingVbkBlock->getHash(), popData.vtbs, state))
      << state.toString();

  // manually add duplicated VTBs
  auto* containingVbkBlock_index =
      alttree.vbk().getBlockIndex(containingVbkBlock->getHash());
  containingVbkBlock_index->insertPayloadId<VTB>(vtb1.getId());
  alttree.getPayloadsIndex().addVbkPayloadIndex(
      containingVbkBlock_index->getHash(), vtb1.getId().asVector());

  ASSERT_DEATH(save(alttree), "");
}

void emptyValidator(const BlockIndex<VbkBlock>&) {}

TEST_F(SaveLoadTreeTest, ReloadWithDuplicatesVbk_test2) {
  PopData popData;

  auto vbkPopTx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      popminer->vbkTip()->getHeader(), popminer->btcTip()->getHash());

  // both VTBs should be contained in the same block
  BlockIndex<VbkBlock>* containingVbkBlock =
      popminer->mineVbkBlocks(1, {vbkPopTx1});

  auto vtb1 = popminer->createVTB(containingVbkBlock->getHeader(), vbkPopTx1);
  popData.vtbs = {vtb1};
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock->getHash(),
                 popminer->vbk());
  payloadsProvider.writePayloads(popData);

  // manually add the VBK context to alttree
  for (auto& block : popData.context) {
    ASSERT_TRUE(alttree.vbk().acceptBlockHeader(block, state))
        << state.toString();
  }

  ASSERT_TRUE(alttree.vbk().addPayloads(
      containingVbkBlock->getHash(), popData.vtbs, state))
      << state.toString();

  // manually add duplicated VTBs
  auto* containingVbkBlock_index =
      alttree.vbk().getBlockIndex(containingVbkBlock->getHash());
  containingVbkBlock_index->insertPayloadId<VTB>(vtb1.getId());
  alttree.getPayloadsIndex().addVbkPayloadIndex(
      containingVbkBlock_index->getHash(), vtb1.getId().asVector());

  auto batch = storage.generateWriteBatch();
  auto writer = adaptors::BlockBatchImpl(*batch);
  saveTree(alttree.btc(), writer);
  saveTree(alttree.vbk(), writer, emptyValidator);
  saveTree(alttree, writer);
  batch->writeBatch();

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "failed-to-load-vbk-tree+load-tree+VTB-duplicate");
}

TEST_F(SaveLoadTreeTest, SaveUpdatedBlock_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  auto* endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  ASSERT_TRUE(endorsedIndex->isDirty());

  save(alttree);

  endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  ASSERT_FALSE(endorsedIndex->isDirty());

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData popData = generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  endorsedIndex = alttree.getBlockIndex(endorsedBlock.getHash());
  ASSERT_TRUE(endorsedIndex->isDirty());

  ASSERT_TRUE(load(alttree2)) << state.toString();
}
