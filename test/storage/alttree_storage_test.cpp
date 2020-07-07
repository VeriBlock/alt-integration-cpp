// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/inmem/storage_manager_inmem.hpp>
#include <veriblock/storage/rocks/storage_manager_rocks.hpp>

using namespace altintegration;

static const std::string dbName = "db-test";
static const std::string dbName2 = "db-test2";

struct TestStorageInmem {
  TestStorageInmem() {
    StorageManagerInmem storageManager{};
    storage = std::make_shared<PopStorage>(storageManager.getPopStorage());
    storagePayloads2 = std::make_shared<PayloadsStorageInmem>();
  }

  void saveToPayloadsStorageVbk(
      const PayloadsStorage& from, PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
  }

  void saveToPayloadsStorageAlt(
      const PayloadsStorage& from, PayloadsStorage& to) {
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
    storageManager = std::make_shared<StorageManagerRocks>(dbName);
    storage = std::make_shared<PopStorage>(storageManager->getPopStorage());

    storage->getBlockRepo<BlockIndex<BtcBlock>>().clear();
    storage->getBlockRepo<BlockIndex<VbkBlock>>().clear();
    storage->getBlockRepo<BlockIndex<AltBlock>>().clear();
    storage->getEndorsementsRepo<VbkEndorsement>().clear();
    storage->getEndorsementsRepo<AltEndorsement>().clear();

    storageManager2 = std::make_shared<StorageManagerRocks>(dbName2);
    storagePayloads2 =
        std::make_shared<PayloadsStorage>(storageManager->getPayloadsStorage());

    storagePayloads2->getRepo<ATV>().clear();
    storagePayloads2->getRepo<VTB>().clear();
    storagePayloads2->getRepo<VbkBlock>().clear();
  }

  void saveToPayloadsStorageVbk(
      const PayloadsStorage& from, PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
  }

  void saveToPayloadsStorageAlt(
      const PayloadsStorage& from, PayloadsStorage& to) {
    payloadsRepositoryCopy(from.getRepo<ATV>(), to.getRepo<ATV>());
    payloadsRepositoryCopy(from.getRepo<VTB>(), to.getRepo<VTB>());
    payloadsRepositoryCopy(from.getRepo<VbkBlock>(), to.getRepo<VbkBlock>());
  }

  std::shared_ptr<PopStorage> storage;
  std::shared_ptr<StorageManager> storageManager;
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

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TYPED_TEST_SUITE_P(AltTreeRepositoryTest);

TYPED_TEST_P(AltTreeRepositoryTest, Basic) {
  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx = this->popminer->createBtcTxEndorsingVbkBlock(*vbkTip->header);
  // add BTC tx endorsing VBKTIP into next block
  auto* chainAtip = this->popminer->mineBtcBlocks(1);

  // create VBK pop tx that has 'block of proof=CHAIN A'
  auto txa = this->popminer->createVbkPopTxEndorsingVbkBlock(
      *chainAtip->header,
      btctx,
      *vbkTip->header,
      lastKnownLocalBtcBlock(*this->popminer));
  // erase part of BTC - it will be restored from payloads anyway
  this->popminer->btc().removeLeaf(*this->popminer->btc().getBestChain().tip(),
                                   true);

  // mine txA into VBK 2nd block
  vbkTip = this->popminer->mineVbkBlocks(1);

  EXPECT_TRUE(this->popminer->vbk().saveToStorage(*this->storage, this->state));
  this->saveToPayloadsStorageVbk(this->popminer->vbk().getStoragePayloads(),
                                 *this->storagePayloads2);
  VbkBlockTree reloadedVbkTree{
      this->vbkparam, this->btcparam, *this->storagePayloads2};
  EXPECT_TRUE(reloadedVbkTree.loadFromStorage(*this->storage, this->state));

  EXPECT_TRUE(reloadedVbkTree.btc() == this->popminer->btc());
  EXPECT_TRUE(reloadedVbkTree == this->popminer->vbk());

  this->popminer->vbk().removeLeaf(*this->popminer->vbk().getBestChain().tip(),
                                   true);
  EXPECT_FALSE(reloadedVbkTree == this->popminer->vbk());

  // commands should be properly restored to make it pass
  reloadedVbkTree.removeLeaf(*reloadedVbkTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedVbkTree == this->popminer->vbk());
  EXPECT_TRUE(reloadedVbkTree.btc() == this->popminer->btc());
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

  EXPECT_TRUE(this->alttree.saveToStorage(*this->storage, this->state));
  this->saveToPayloadsStorageVbk(this->alttree.vbk().getStoragePayloads(),
                                 *this->storagePayloads2);
  this->saveToPayloadsStorageAlt(this->alttree.getStoragePayloads(),
                                 *this->storagePayloads2);

  AltTree reloadedAltTree{this->altparam, this->vbkparam, this->btcparam, *this->storagePayloads2};
  EXPECT_TRUE(reloadedAltTree.loadFromStorage(*this->storage, this->state));

  EXPECT_TRUE(reloadedAltTree.vbk().btc() == this->alttree.vbk().btc());
  EXPECT_TRUE(reloadedAltTree.vbk() == this->alttree.vbk());
  EXPECT_TRUE(reloadedAltTree == this->alttree);

  this->alttree.removeLeaf(*this->alttree.getBestChain().tip(), true);
  EXPECT_FALSE(reloadedAltTree == this->alttree);

  reloadedAltTree.removeLeaf(*reloadedAltTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedAltTree == this->alttree);
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(AltTreeRepositoryTest, Basic, Altchain);

typedef ::testing::Types<TestStorageInmem, TestStorageRocks> TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(AltTreeRepositoryTestSuite,
                               AltTreeRepositoryTest,
                               TypesUnderTest);
