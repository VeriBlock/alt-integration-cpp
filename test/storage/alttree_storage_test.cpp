// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/block_repository_rocks.hpp>
#include <veriblock/storage/tips_repository_inmem.hpp>
#include <veriblock/storage/tips_repository_rocks.hpp>
#include <veriblock/storage/payloads_repository_inmem.hpp>
#include <veriblock/storage/payloads_repository_rocks.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>
#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>
#include <veriblock/storage/pop_storage.hpp>

using namespace altintegration;

static const std::string dbName = "db-test";

struct PopStorageInmem {
  PopStorageInmem() {
    auto repoBtc =
        std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    auto repoVbk =
        std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    auto repoAlt =
        std::make_shared<BlockRepositoryInmem<BlockIndex<AltBlock>>>();
    auto repoTipsBtc =
        std::make_shared<TipsRepositoryInmem<BlockIndex<BtcBlock>>>();
    auto repoTipsVbk =
        std::make_shared<TipsRepositoryInmem<BlockIndex<VbkBlock>>>();
    auto repoTipsAlt =
        std::make_shared<TipsRepositoryInmem<BlockIndex<AltBlock>>>();
    auto erepoVbk = std::make_shared<PayloadsRepositoryInmem<VbkEndorsement>>();
    auto erepoAlt = std::make_shared<PayloadsRepositoryInmem<AltEndorsement>>();

    storage = std::make_shared<PopStorage>(repoBtc,
                                           repoVbk,
                                           repoAlt,
                                           repoTipsBtc,
                                           repoTipsVbk,
                                           repoTipsAlt,
                                           erepoVbk,
                                           erepoAlt);
    storage->getBlockRepo<BlockIndex<BtcBlock>>().clear();
    storage->getBlockRepo<BlockIndex<VbkBlock>>().clear();
    storage->getBlockRepo<BlockIndex<AltBlock>>().clear();
    storage->getEndorsementsRepo<VbkEndorsement>().clear();
    storage->getEndorsementsRepo<AltEndorsement>().clear();
  }

  std::shared_ptr<PopStorage> storage;
};

struct PopStorageRocks {
  PopStorageRocks() {
    database = std::make_shared<RepositoryRocksManager>(dbName);
    database->attachColumn("btc_blocks");
    database->attachColumn("vbk_blocks");
    database->attachColumn("alt_blocks");
    database->attachColumn("tips");
    database->attachColumn("vbk_endorsements");
    database->attachColumn("alt_endorsements");
    EXPECT_EQ(database->open(), rocksdb::Status::OK());
    EXPECT_EQ(database->clear(), rocksdb::Status::OK());

    auto* db = database->getDB();
    auto* column = database->getColumn("btc_blocks");
    auto repoBtc = std::make_shared<BlockRepositoryRocks<BlockIndex<BtcBlock>>>(
        db, column);
    column = database->getColumn("vbk_blocks");
    auto repoVbk = std::make_shared<BlockRepositoryRocks<BlockIndex<VbkBlock>>>(
        db, column);
    column = database->getColumn("alt_blocks");
    auto repoAlt = std::make_shared<BlockRepositoryRocks<BlockIndex<AltBlock>>>(
        db, column);
    column = database->getColumn("tips");
    auto repoTipsBtc =
        std::make_shared<TipsRepositoryRocks<BlockIndex<BtcBlock>>>(db, column);
    auto repoTipsVbk =
        std::make_shared<TipsRepositoryRocks<BlockIndex<VbkBlock>>>(db, column);
    auto repoTipsAlt =
        std::make_shared<TipsRepositoryRocks<BlockIndex<AltBlock>>>(db, column);
    column = database->getColumn("vbk_endorsements");
    auto erepoVbk =
        std::make_shared<PayloadsRepositoryRocks<VbkEndorsement>>(db, column);
    column = database->getColumn("alt_endorsements");
    auto erepoAlt =
        std::make_shared<PayloadsRepositoryRocks<AltEndorsement>>(db, column);

    storage = std::make_shared<PopStorage>(repoBtc,
                                           repoVbk,
                                           repoAlt,
                                           repoTipsBtc,
                                           repoTipsVbk,
                                           repoTipsAlt,
                                           erepoVbk,
                                           erepoAlt);
    storage->getBlockRepo<BlockIndex<BtcBlock>>().clear();
    storage->getBlockRepo<BlockIndex<VbkBlock>>().clear();
    storage->getBlockRepo<BlockIndex<AltBlock>>().clear();
    storage->getEndorsementsRepo<VbkEndorsement>().clear();
    storage->getEndorsementsRepo<AltEndorsement>().clear();
  }

  std::shared_ptr<PopStorage> storage;
  std::shared_ptr<RepositoryRocksManager> database;
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
  this->popminer->btc().removeTip(*this->popminer->btc().getBestChain().tip(),
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

  this->popminer->vbk().removeTip(*this->popminer->vbk().getBestChain().tip(),
                                  true);
  EXPECT_FALSE(reloadedVbkTree == this->popminer->vbk());

  // commands should be properly restored to make it pass
  reloadedVbkTree.removeTip(*reloadedVbkTree.getBestChain().tip(), true);
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

  this->alttree.removeTip(*this->alttree.getBestChain().tip(), true);
  EXPECT_FALSE(reloadedAltTree == this->alttree);

  reloadedAltTree.removeTip(*reloadedAltTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedAltTree == this->alttree);
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(AltTreeRepositoryTest, Basic, Altchain);

typedef ::testing::Types<PopStorageInmem, PopStorageRocks>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(AltTreeRepositoryTestSuite,
                               AltTreeRepositoryTest,
                               TypesUnderTest);
