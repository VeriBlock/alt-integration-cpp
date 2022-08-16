// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

struct SaveLoadTreeTest : public PopTestFixture, public testing::Test {
  SaveLoadTreeTest() {
    alttree2.btc().bootstrapWithGenesis(GetRegTestBtcBlock());
    alttree2.vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
    alttree2.bootstrap();

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

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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

  save(alttree);

  EXPECT_TRUE(load(alttree2));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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

  const auto& pl = alttree.getPayloadsIndex();
  auto& plmut = as_mut(pl);
  for (const auto& b : popData.context) {
    plmut.add(b.getId().asVector(), containingBlock.getHash());
  }

  save(alttree);

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(),
            "failed-to-load-alt-tree+load-tree+valid-block-with-stateful-"
            "duplicates+VBK-duplicate");
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test2) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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
    as_mut(alttree.getPayloadsIndex())
        .add(b.getId().asVector(), containingBlock.getHash());
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

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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
  as_mut(alttree.getPayloadsIndex())
      .add(popData.atvs[0].getId().asVector(), containingBlock.getHash());

  save(alttree);

  EXPECT_FALSE(load(alttree2));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(),
            "failed-to-load-alt-tree+load-tree+valid-block-with-stateful-"
            "duplicates+ATV-duplicate");
}

TEST_F(SaveLoadTreeTest, ReloadWithDuplicatesVbk_test1) {
  PopData popData;

  auto vbkPopTx1 = popminer.createVbkPopTxEndorsingVbkBlock(
      popminer.vbkTip()->getHeader(), popminer.btcTip()->getHash());

  // both VTBs should be contained in the same block
  BlockIndex<VbkBlock>* containingVbkBlock =
      popminer.mineVbkBlocks(1, {vbkPopTx1});

  auto vtb1 = popminer.createVTB(containingVbkBlock->getHeader(), vbkPopTx1);
  popData.vtbs = {vtb1};
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock->getHash(),
                 popminer.vbk());
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
  as_mut(alttree.vbk().getPayloadsIndex())
      .add(vtb1.getId().asVector(), containingVbkBlock_index->getHash());

  ASSERT_DEATH(save(alttree), "");
}

void emptyValidator(const BlockIndex<VbkBlock>&) {}

TEST_F(SaveLoadTreeTest, ReloadWithDuplicatesVbk_test2) {
  PopData popData;

  auto vbkPopTx1 = popminer.createVbkPopTxEndorsingVbkBlock(
      popminer.vbkTip()->getHeader(), popminer.btcTip()->getHash());

  // both VTBs should be contained in the same block
  BlockIndex<VbkBlock>* containingVbkBlock =
      popminer.mineVbkBlocks(1, {vbkPopTx1});

  auto vtb1 = popminer.createVTB(containingVbkBlock->getHeader(), vbkPopTx1);
  popData.vtbs = {vtb1};
  fillVbkContext(popData.context,
                 alttree.vbk().getBestChain().tip()->getHash(),
                 containingVbkBlock->getHash(),
                 popminer.vbk());
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
  as_mut(alttree.vbk().getPayloadsIndex())
      .add(vtb1.getId().asVector(), containingVbkBlock_index->getHash());

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

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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

template <typename TreeT>
void assertTreeNotDirty(const TreeT& tree) {
  const auto& blocks = tree.getBlocks();
  for (const auto* b : blocks) {
    ASSERT_FALSE(b->isDirty());
  }
}

void assertAllTreesNotDirty(const AltBlockTree& tree) {
  assertTreeNotDirty(tree);
  assertTreeNotDirty(tree.btc());
  assertTreeNotDirty(tree.vbk());
}

TEST_F(SaveLoadTreeTest, CheckForDirtyBlocks_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock altBlock = chain[5];

  auto* altBlockIndex = alttree.getBlockIndex(altBlock.getHash());
  ASSERT_TRUE(altBlockIndex->isDirty());

  save(alttree);

  altBlockIndex = alttree.getBlockIndex(altBlock.getHash());
  ASSERT_FALSE(altBlockIndex->isDirty());
  ASSERT_TRUE(load(alttree2)) << state.toString();

  assertAllTreesNotDirty(alttree);
  assertAllTreesNotDirty(alttree2);
}

TEST_F(SaveLoadTreeTest, CheckForDirtyEndorsedBlocks_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
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

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  alttree.acceptBlock(containingBlock.getHash(), {});

  save(alttree);

  EXPECT_TRUE(load(alttree2));
  EXPECT_TRUE(state.IsValid());

  assertAllTreesNotDirty(alttree);
  assertAllTreesNotDirty(alttree2);
}
