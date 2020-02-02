#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository_rocks.hpp"
#include "veriblock/storage/stored_btcblock.hpp"
#include "veriblock/storage/stored_vbkblock.hpp"

using namespace VeriBlock;

static const BtcBlock defaultBlockBtc{
    536870912u,
    "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
    1555501858u,
    436279940u,
    (uint32_t)-1695416274};

static const VbkBlock defaultBlockVbk{5000,
                                      2,
                                      "449c60619294546ad825af03"_unhex,
                                      "b0935637860679ddd5"_unhex,
                                      "5ee4fd21082e18686e"_unhex,
                                      "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
                                      1553699059,
                                      16842752,
                                      1};

static rocksdb::Status openDB(
    rocksdb::DB **db, std::vector<rocksdb::ColumnFamilyHandle *> &cfHandles) {
  ///TODO: erase DB before opening otherwise it may fail if scheme was changed

  // prepare column families
  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
  column_families.push_back(rocksdb::ColumnFamilyDescriptor(
      rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions()));
  column_families.push_back(rocksdb::ColumnFamilyDescriptor(
      "height_hashes_btc", rocksdb::ColumnFamilyOptions()));
  column_families.push_back(rocksdb::ColumnFamilyDescriptor(
      "hash_block_btc", rocksdb::ColumnFamilyOptions()));
  column_families.push_back(rocksdb::ColumnFamilyDescriptor(
      "height_hashes_vbk", rocksdb::ColumnFamilyOptions()));
  column_families.push_back(rocksdb::ColumnFamilyDescriptor(
      "hash_block_vbk", rocksdb::ColumnFamilyOptions()));
  rocksdb::Options options;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  return rocksdb::DB::Open(options, "db-test", column_families, &cfHandles, db);
}

TEST(Storage, ConfigTest) {
  rocksdb::DB *db = 0;
  std::vector<rocksdb::ColumnFamilyHandle *> cfHandles;
  rocksdb::Status s = openDB(&db, cfHandles);
  ASSERT_TRUE(s.ok());

  auto dbPtr = std::shared_ptr<rocksdb::DB>(db);
  // make sure we save a pointer to default handle. We should
  // destroy it properly otherwise we will see assertion failed
  auto defaultHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[0]);
  auto heightHashesBtcHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[1]);
  auto hashBlockBtcHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[2]);
  auto heightHashesVbkHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[3]);
  auto hashBlockVbkHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[4]);
  BlockRepositoryRocks<StoredBtcBlock> repoBtc(
      dbPtr, hashBlockBtcHandlePtr, heightHashesBtcHandlePtr);
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(defaultBlockBtc, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);

  BlockRepositoryRocks<StoredVbkBlock> repoVbk(
      dbPtr, hashBlockVbkHandlePtr, heightHashesVbkHandlePtr);
  StoredVbkBlock blockVbk = StoredVbkBlock::fromBlock(defaultBlockVbk);
  bool retVbk = repoVbk.put(blockVbk);
  ASSERT_TRUE(retVbk);
}

TEST(Storage, PutAndGet) {
  rocksdb::DB *db = 0;
  std::vector<rocksdb::ColumnFamilyHandle *> cfHandles;
  rocksdb::Status s = openDB(&db, cfHandles);
  ASSERT_TRUE(s.ok());

  auto dbPtr = std::shared_ptr<rocksdb::DB>(db);
  // make sure we save a pointer to default handle. We should
  // destroy it properly otherwise we will see assertion failed
  auto defaultHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[0]);
  auto heightHashesBtcHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[1]);
  auto hashBlockBtcHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[2]);
  auto heightHashesVbkHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[3]);
  auto hashBlockVbkHandlePtr =
      std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandles[4]);
  BlockRepositoryRocks<StoredBtcBlock> repoBtc(
      dbPtr, hashBlockBtcHandlePtr, heightHashesBtcHandlePtr);
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(defaultBlockBtc, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);

  StoredBtcBlock readBlock;
  bool readResult = repoBtc.getByHash(defaultBlockBtc.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  EXPECT_EQ(blockBtc.height, readBlock.height);
  EXPECT_EQ(blockBtc.hash, readBlock.hash);
  EXPECT_EQ(blockBtc.block.version, readBlock.block.version);
}
