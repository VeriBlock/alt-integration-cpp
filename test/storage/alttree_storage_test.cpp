// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/storage_manager.hpp>
#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>

using namespace altintegration;

static const std::string dbName = "db-test";

struct PopStorageInmem {
  PopStorageInmem() {
    StorageManager storageManager{};
    storage = std::make_shared<PopStorage>(storageManager.newPopStorageInmem());
  }

  std::shared_ptr<PopStorage> storage;
};

struct PopStorageRocks {
  PopStorageRocks() {
    storageManager = std::make_shared<StorageManager>(dbName);
    storageManager->openRocks();
    storageManager->clearRocks();
    storage =
        std::make_shared<PopStorage>(storageManager->newPopStorageRocks());

    storage->getBlockRepo<BlockIndex<BtcBlock>>().clear();
    storage->getBlockRepo<BlockIndex<VbkBlock>>().clear();
    storage->getBlockRepo<BlockIndex<AltBlock>>().clear();
    storage->getEndorsementsRepo<VbkEndorsement>().clear();
    storage->getEndorsementsRepo<AltEndorsement>().clear();
  }

  std::shared_ptr<PopStorage> storage;
  std::shared_ptr<StorageManager> storageManager;
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

  VbkBlockTree reloadedVbkTree{this->vbkparam,
                               this->btcparam,
                               this->popminer->vbk().getStoragePayloads()};
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

  AltTree reloadedAltTree{this->altparam,
                          this->vbkparam,
                          this->btcparam,
                          this->alttree.getStoragePayloads()};
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

typedef ::testing::Types<PopStorageInmem, PopStorageRocks>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(AltTreeRepositoryTestSuite,
                               AltTreeRepositoryTest,
                               TypesUnderTest);
