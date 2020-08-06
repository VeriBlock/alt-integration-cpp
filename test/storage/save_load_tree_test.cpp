// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/pop_storage_batch_adaptor.hpp>

using namespace altintegration;

struct SaveLoadTreeTest : public PopTestFixture, public testing::Test {
  SaveLoadTreeTest() {
    alttree2.btc().bootstrapWithGenesis(state);
    alttree2.vbk().bootstrapWithGenesis(state);
    alttree2.bootstrap(state);

    chain.push_back(altparam.getBootstrapBlock());
    createEndorsedAltChain(20, 3);
  }

  std::vector<AltBlock> chain;
  PopStorageInmem storage;
  StorageManagerInmem storageManager2{};
  PayloadsStorage& storagePayloads2 = storageManager2.getPayloadsStorage();

  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam, storagePayloads2);

  void save() {
    auto adaptor = PopStorageBatchAdaptor(storage);
    SaveAllTrees(alttree, adaptor);
  }

  bool load(ValidationState& _state) {
    // copy payloads from storagePayloads -> storagePayloads2
    auto cursor = storagePayloads.getRepo().newCursor();
    auto batch = storagePayloads2.getRepo().newBatch();
    for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
      batch->put(cursor->key(), cursor->value());
    }
    batch->commit();

    // now load blocks
    return LoadTreeWrapper(alttree2.btc(), storage, _state) &&
           LoadTreeWrapper(alttree2.vbk(), storage, _state) &&
           LoadTreeWrapper(alttree2, storage, _state);
  }

  auto assertTreesEqual() {
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
  save();
  ASSERT_TRUE(load(state));
  assertTreesEqual();
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData popData =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, popData, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  containingIndex->template insertPayloadIds<VbkBlock>(
      map_get_id(popData.context));

  for (const auto& b : popData.context) {
    alttree.getStorage().addAltPayloadIndex(containingBlock.getHash(),
                                            b.getId().asVector());
  }

  save();

  EXPECT_FALSE(load(state));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "load-tree+ALT-duplicate-payloads-VBK");
}

TEST_F(SaveLoadTreeTest, ReloadWithoutDuplicates_test2) {
  // mine 20 blocks
  mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData popData =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, popData, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  containingIndex->template insertPayloadIds<ATV>(map_get_id(popData.atvs));

  // add duplicates
  // add duplicate atv
  alttree.getStorage().addAltPayloadIndex(containingBlock.getHash(),
                                          popData.atvs[0].getId().asVector());

  save();

  EXPECT_FALSE(load(state));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "load-tree+ALT-duplicate-payloads-ATV-ancestor");
}
