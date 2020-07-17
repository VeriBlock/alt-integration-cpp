// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/inmem/storage_manager_inmem.hpp>
#include <veriblock/storage/repo_batch_adaptor.hpp>
#include <veriblock/storage/rocks/storage_manager_rocks.hpp>
#include <veriblock/storage/util.hpp>

using namespace altintegration;

static const std::string dbName = "db-test";
static const std::string dbName2 = "db-test2";

struct TestStorageInmem {
  TestStorageInmem() {
    StorageManagerInmem storageManager{};
    storage = std::make_shared<PopStorage>(storageManager.getPopStorage());
    storagePayloads2 = std::make_shared<PayloadsStorageInmem>();
  }

  void saveToPayloadsStorageVbk(const PayloadsStorage& from,
                                PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
  }

  void saveToPayloadsStorageAlt(const PayloadsStorage& from,
                                PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<ATV>(), to.getRepo<ATV>());
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
    payloadsRepositoryCopy(from.getRepo<VbkBlock>(), to.getRepo<VbkBlock>());
  }

  std::shared_ptr<PopStorage> storage;
  // another DB instance for the data copy
  std::shared_ptr<PayloadsStorage> storagePayloads2;
};

struct TestStorageRocks {
  TestStorageRocks() {
    storageManager_ = std::make_shared<StorageManagerRocks>(dbName);
    storage = std::make_shared<PopStorage>(storageManager_->getPopStorage());

    storage->getBlockRepo<BlockIndex<BtcBlock>>().clear();
    storage->getBlockRepo<BlockIndex<VbkBlock>>().clear();
    storage->getBlockRepo<BlockIndex<AltBlock>>().clear();

    storageManager2 = std::make_shared<StorageManagerRocks>(dbName2);
    storagePayloads2 = std::make_shared<PayloadsStorage>(
        storageManager_->getPayloadsStorage());

    storagePayloads2->getRepo<ATV>().clear();
    storagePayloads2->getRepo<VTB>().clear();
    storagePayloads2->getRepo<VbkBlock>().clear();
  }

  void saveToPayloadsStorageVbk(const PayloadsStorage& from,
                                PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
  }

  void saveToPayloadsStorageAlt(const PayloadsStorage& from,
                                PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<ATV>(), to.getRepo<ATV>());
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
    payloadsRepositoryCopy(from.getRepo<VbkBlock>(), to.getRepo<VbkBlock>());
  }

  std::shared_ptr<PopStorage> storage;
  std::shared_ptr<StorageManager> storageManager_;
  // another DB instance for the data copy
  std::shared_ptr<PayloadsStorage> storagePayloads2;
  std::shared_ptr<StorageManager> storageManager2;
};

template <typename Storage>
struct AltTreeRepositoryTest : public ::testing::Test,
                               public PopTestFixture,
                               public Storage {
  AltTreeRepositoryTest() {}
};

template <typename index_t>
std::vector<std::pair<int, index_t>> LoadBlocksFromDisk(PopStorage& storage) {
  auto map = storage.loadBlocks<index_t>();
  std::vector<std::pair<int, index_t>> ret;
  for (auto& pair : map) {
    ret.push_back({pair.first, *pair.second});
  }

  std::sort(ret.begin(),
            ret.end(),
            [](const std::pair<int, index_t>& a,
               const std::pair<int, index_t>& b) { return a.first < b.first; });
  return ret;
}

template <typename index_t>
typename index_t::hash_t LoadTipFromDisk(PopStorage& storage) {
  auto tip = storage.loadTip<index_t>();
  return tip.second;
}

template <typename Tree>
bool LoadTreeWrapper(Tree& tree, PopStorage& storage, ValidationState& state) {
  using index_t = typename Tree::index_t;
  auto blocks = LoadBlocksFromDisk<index_t>(storage);
  auto tip = LoadTipFromDisk<index_t>(storage);
  return LoadTree<Tree>(tree, blocks, tip, state);
}

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TYPED_TEST_SUITE_P(AltTreeRepositoryTest);

TYPED_TEST_P(AltTreeRepositoryTest, Basic) {
  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx = this->popminer->createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
  // add BTC tx endorsing VBKTIP into next block
  auto* chainAtip = this->popminer->mineBtcBlocks(1);

  // create VBK pop tx that has 'block of proof=CHAIN A'
  this->popminer->createVbkPopTxEndorsingVbkBlock(
      chainAtip->getHeader(),
      btctx,
      vbkTip->getHeader(),
      lastKnownLocalBtcBlock(*this->popminer));
  // erase part of BTC - it will be restored from payloads anyway
  this->popminer->btc().removeLeaf(*this->popminer->btc().getBestChain().tip());

  // mine txA into VBK 2nd block
  vbkTip = this->popminer->mineVbkBlocks(1);

  auto adaptor = RepoBatchAdaptor(*this->storage);
  SaveTree(this->popminer->btc(), adaptor);
  SaveTree(this->popminer->vbk(), adaptor);
  this->saveToPayloadsStorageVbk(this->popminer->vbk().getStoragePayloads(),
                                 *this->storagePayloads2);
  VbkBlockTree newvbk{this->vbkparam, this->btcparam, *this->storagePayloads2};
  newvbk.btc().bootstrapWithGenesis(this->state);
  newvbk.bootstrapWithGenesis(this->state);

  ASSERT_TRUE(LoadTreeWrapper(newvbk.btc(), *this->storage, this->state))
      << this->state.toString();
  ASSERT_TRUE(LoadTreeWrapper(newvbk, *this->storage, this->state))
      << this->state.toString();

  ASSERT_EQ(newvbk.btc(), this->popminer->btc());
  ASSERT_EQ(newvbk, this->popminer->vbk());
  this->popminer->vbk().removeLeaf(*this->popminer->vbk().getBestChain().tip());
  ASSERT_FALSE(newvbk == this->popminer->vbk());

  // commands should be properly restored to make it pass
  newvbk.removeLeaf(*newvbk.getBestChain().tip());
  ASSERT_TRUE(newvbk == this->popminer->vbk());
  ASSERT_TRUE(newvbk.btc() == this->popminer->btc());
}

TYPED_TEST_P(AltTreeRepositoryTest, Altchain) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 2 blocks
  this->mineAltBlocks(2, chain);

  AltBlock endorsedBlock = chain[2];

  VbkTx tx = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock));
  AltBlock containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData altPayloads1 = this->generateAltPayloads(
      {tx}, this->vbkparam.getGenesisBlock().getHash());

  // mine 1 VBK blocks
  this->popminer->mineVbkBlocks(1);
  this->popminer->mineBtcBlocks(1);

  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));
  EXPECT_TRUE(
      this->alttree.addPayloads(containingBlock, altPayloads1, this->state));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());

  auto adaptor = RepoBatchAdaptor(*this->storage);
  SaveAllTrees(this->alttree, adaptor);
  this->saveToPayloadsStorageVbk(this->alttree.vbk().getStoragePayloads(),
                                 *this->storagePayloads2);
  this->saveToPayloadsStorageAlt(this->alttree.getStoragePayloads(),
                                 *this->storagePayloads2);

  AltTree reloadedAltTree{
      this->altparam, this->vbkparam, this->btcparam, *this->storagePayloads2};

  reloadedAltTree.btc().bootstrapWithGenesis(this->state);
  reloadedAltTree.vbk().bootstrapWithGenesis(this->state);
  reloadedAltTree.bootstrap(this->state);

  ASSERT_TRUE(
      LoadTreeWrapper(reloadedAltTree.btc(), *this->storage, this->state));
  ASSERT_TRUE(
      LoadTreeWrapper(reloadedAltTree.vbk(), *this->storage, this->state));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree, *this->storage, this->state));

  ASSERT_EQ(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc());
  ASSERT_EQ(reloadedAltTree.vbk(), this->alttree.vbk());
  ASSERT_EQ(reloadedAltTree, this->alttree);

  this->alttree.removeLeaf(*this->alttree.getBestChain().tip());
  EXPECT_FALSE(reloadedAltTree == this->alttree);

  reloadedAltTree.removeLeaf(*reloadedAltTree.getBestChain().tip());
  EXPECT_TRUE(reloadedAltTree == this->alttree);
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(AltTreeRepositoryTest, Basic, Altchain);

typedef ::testing::Types<TestStorageInmem, TestStorageRocks> TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(AltTreeRepositoryTestSuite,
                               AltTreeRepositoryTest,
                               TypesUnderTest);
