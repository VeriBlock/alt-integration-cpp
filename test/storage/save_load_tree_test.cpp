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
    ASSERT_EQ(alttree, alttree2);
    auto tip = alttree.getBestChain().tip();
    auto to1 = alttree.getBestChain().first()->getHash();
    EXPECT_TRUE(alttree.setState(to1, state));
    EXPECT_TRUE(alttree2.setState(to1, state));
    ASSERT_EQ(alttree, alttree2);
    auto to2 = tip->getHash();
    EXPECT_TRUE(alttree.setState(to2, state));
    EXPECT_TRUE(alttree2.setState(to2, state));
    ASSERT_EQ(alttree, alttree2);
  }
};

// alttree does not contain any invalid blocks
TEST_F(SaveLoadTreeTest, ValidTree) {
  save();
  ASSERT_TRUE(load(state));
  assertTreesEqual();
}