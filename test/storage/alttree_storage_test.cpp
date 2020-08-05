// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/inmem/storage_manager_inmem.hpp>
#include <veriblock/storage/pop_storage_batch_adaptor.hpp>
#include <veriblock/storage/util.hpp>

#ifdef VERIBLOCK_ROCKSDB
#include <veriblock/storage/rocks/storage_manager_rocks.hpp>
#endif  // VERIBLOCK_ROCKSDB

using namespace altintegration;

static const std::string dbName = "db-test";
static const std::string dbName2 = "db-test2";

struct TestStorageInmem {
  TestStorageInmem() {
    StorageManagerInmem storageManager{};
    storage = std::make_shared<PopStorageInmem>();
    storagePayloads2 = std::make_shared<PayloadsStorageInmem>();
  }

  void repositoryCopy(const Repository& copyFrom, Repository& copyTo) {
    auto cursor = copyFrom.newCursor();
    for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
      copyTo.put(cursor->key(), cursor->value());
    }
  }

  void saveToPayloadsStorage(const PayloadsStorage& from, PayloadsStorage& to) {
    repositoryCopy(from.getRepo(), to.getRepo());
  }

  std::shared_ptr<PopStorage> storage;
  // another DB instance for the data copy
  std::shared_ptr<PayloadsStorage> storagePayloads2;
};

#ifdef VERIBLOCK_ROCKSDB

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

#endif  // VERIBLOCK_ROCKSDB

template <typename Storage>
struct AltTreeRepositoryTest : public ::testing::Test,
                               public PopTestFixture,
                               public Storage {
  AltTreeRepositoryTest() {}
};

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TYPED_TEST_SUITE_P(AltTreeRepositoryTest);

TYPED_TEST_P(AltTreeRepositoryTest, ValidBlocks) {
  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx =
      this->popminer->createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
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

  auto adaptor = PopStorageBatchAdaptor(*this->storage);
  SaveTree(this->popminer->btc(), adaptor);
  SaveTree(this->popminer->vbk(), adaptor);
  this->saveToPayloadsStorage(this->popminer->vbk().getStorage(),
                              *this->storagePayloads2);
  VbkBlockTree newvbk{this->vbkparam, this->btcparam, *this->storagePayloads2};
  newvbk.btc().bootstrapWithGenesis(this->state);
  newvbk.bootstrapWithGenesis(this->state);

  ASSERT_TRUE(LoadTreeWrapper(newvbk.btc(), *this->storage, this->state))
      << this->state.toString();
  ASSERT_TRUE(LoadTreeWrapper(newvbk, *this->storage, this->state))
      << this->state.toString();

  ASSERT_TRUE(this->cmp(newvbk.btc(), this->popminer->btc()));
  ASSERT_TRUE(this->cmp(newvbk, this->popminer->vbk()));
  this->popminer->vbk().removeLeaf(*this->popminer->vbk().getBestChain().tip());
  ASSERT_FALSE(this->cmp(newvbk, this->popminer->vbk(), true));

  // commands should be properly restored to make it pass
  newvbk.removeLeaf(*newvbk.getBestChain().tip());
  ASSERT_TRUE(this->cmp(newvbk, this->popminer->vbk()));
  ASSERT_TRUE(this->cmp(newvbk.btc(), this->popminer->btc()));
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

  auto adaptor = PopStorageBatchAdaptor(*this->storage);
  SaveAllTrees(this->alttree, adaptor);
  this->saveToPayloadsStorage(this->alttree.vbk().getStorage(),
                              *this->storagePayloads2);
  this->saveToPayloadsStorage(this->alttree.getStorage(),
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

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));

  this->alttree.removeLeaf(*this->alttree.getBestChain().tip());
  EXPECT_FALSE(this->cmp(reloadedAltTree, this->alttree, true));

  reloadedAltTree.removeLeaf(*reloadedAltTree.getBestChain().tip());
  EXPECT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}

TYPED_TEST_P(AltTreeRepositoryTest, ManyEndorsements) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 2 blocks
  this->mineAltBlocks(2, chain);

  AltBlock endorsedBlock1 = chain[1];
  AltBlock endorsedBlock2 = chain[2];

  VbkTx tx1 = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock1));
  VbkTx tx2 = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock2));
  AltBlock containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData altPayloads1 = this->generateAltPayloads(
      {tx1, tx2}, this->vbkparam.getGenesisBlock().getHash());

  // mine 1 VBK blocks
  this->popminer->mineVbkBlocks(1);
  this->popminer->mineBtcBlocks(1);

  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));
  EXPECT_TRUE(
      this->alttree.addPayloads(containingBlock, altPayloads1, this->state));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());

  auto adaptor = PopStorageBatchAdaptor(*this->storage);
  SaveAllTrees(this->alttree, adaptor);
  this->saveToPayloadsStorage(this->alttree.vbk().getStorage(),
                              *this->storagePayloads2);
  this->saveToPayloadsStorage(this->alttree.getStorage(),
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

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}

TYPED_TEST_P(AltTreeRepositoryTest, InvalidBlocks) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 20 blocks
  this->mineAltBlocks(20, chain);

  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx =
      this->popminer->createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
  // add BTC tx endorsing VBKTIP into next block
  auto* chainAtip = this->popminer->mineBtcBlocks(1);

  // create VBK pop tx that has 'block of proof=CHAIN A'
  this->popminer->createVbkPopTxEndorsingVbkBlock(
      chainAtip->getHeader(),
      btctx,
      vbkTip->getHeader(),
      lastKnownLocalBtcBlock(*this->popminer));

  // mine txA into VBK 2nd block
  vbkTip = this->popminer->mineVbkBlocks(1);

  auto vtbs = this->popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  this->fillVbkContext(
      vtbs[0],
      this->popminer->vbk().getParams().getGenesisBlock().getHash(),
      this->popminer->vbk());

  PopData popData = this->createPopData({}, vtbs);
  auto containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  // add alt payloads
  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));
  EXPECT_TRUE(this->alttree.addPayloads(containingBlock, popData, this->state));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());
  validateAlttreeIndexState(this->alttree, containingBlock, popData);

  popData.context.clear();
  // corrupt vtb
  popData.vtbs[0].containingBlock.merkleRoot = uint128();

  containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  // add alt payloads
  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));
  EXPECT_TRUE(this->alttree.addPayloads(containingBlock, popData, this->state));
  EXPECT_FALSE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_FALSE(this->state.IsValid());
  validateAlttreeIndexState(this->alttree, containingBlock, popData, false);

  auto adaptor = PopStorageBatchAdaptor(*this->storage);
  SaveAllTrees(this->alttree, adaptor);
  this->saveToPayloadsStorage(this->alttree.vbk().getStorage(),
                              *this->storagePayloads2);
  this->saveToPayloadsStorage(this->alttree.getStorage(),
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

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));

  // set state that validity flags should be the same
  reloadedAltTree.setState(containingBlock.getHash(), this->state);
  this->alttree.setState(containingBlock.getHash(), this->state);

  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}

TYPED_TEST_P(AltTreeRepositoryTest, ReloadWithoutDuplicates_test) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 20 blocks
  this->mineAltBlocks(20, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock));
  AltBlock containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData popData = this->generateAltPayloads(
      {tx}, this->vbkparam.getGenesisBlock().getHash());
  ASSERT_EQ(popData.atvs.size(), 1);
  ASSERT_EQ(popData.vtbs.size(), 0);

  // add alt payloads
  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));
  EXPECT_TRUE(this->alttree.addPayloads(containingBlock, popData, this->state));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());
  validateAlttreeIndexState(this->alttree, containingBlock, popData);

  containingBlock = this->generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  EXPECT_TRUE(this->alttree.acceptBlock(containingBlock, this->state));

  auto adaptor = PopStorageBatchAdaptor(*this->storage);
  SaveAllTrees(this->alttree, adaptor);
  this->saveToPayloadsStorage(this->alttree.vbk().getStorage(),
                              *this->storagePayloads2);
  this->saveToPayloadsStorage(this->alttree.getStorage(),
                              *this->storagePayloads2);

  // add duplicates
  // add duplicate atv
  this->storagePayloads2->addAltPayloadIndex(
      containingBlock.getHash(), popData.atvs[0].getId().asVector());

  // add duplicate vbk blocks
  for (const auto& b : popData.context) {
    this->storagePayloads2->addAltPayloadIndex(containingBlock.getHash(),
                                               b.getId().asVector());
  }

  AltTree reloadedAltTree{
      this->altparam, this->vbkparam, this->btcparam, *this->storagePayloads2};

  reloadedAltTree.btc().bootstrapWithGenesis(this->state);
  reloadedAltTree.vbk().bootstrapWithGenesis(this->state);
  reloadedAltTree.bootstrap(this->state);

  ASSERT_TRUE(
      LoadTreeWrapper(reloadedAltTree.btc(), *this->storage, this->state));
  ASSERT_TRUE(
      LoadTreeWrapper(reloadedAltTree.vbk(), *this->storage, this->state));
  ASSERT_FALSE(LoadTreeWrapper(reloadedAltTree, *this->storage, this->state));
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(AltTreeRepositoryTest,
                            ValidBlocks,
                            InvalidBlocks,
                            Altchain,
                            ManyEndorsements,
                            ReloadWithoutDuplicates_test);

#ifdef VERIBLOCK_ROCKSDB
typedef ::testing::Types<TestStorageInmem, TestStorageRocks> TypesUnderTest;
#else   //! VERIBLOCK_ROCKSDB
typedef ::testing::Types<TestStorageInmem> TypesUnderTest;
#endif  // VERIBLOCK_ROCKSDB

INSTANTIATE_TYPED_TEST_SUITE_P(AltTreeRepositoryTestSuite,
                               AltTreeRepositoryTest,
                               TypesUnderTest);
