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

enum class CF_NAMES {
  DEFAULT = 0,
  HEIGHT_HASHES_BTC,
  HASH_BLOCK_BTC,
  HEIGHT_HASHES_VBK,
  HASH_BLOCK_VBK
};

// DB name
static const std::string dbName = "db-test";
// this is main DB instance
static rocksdb::DB *dbInstance = nullptr;
static std::vector<rocksdb::ColumnFamilyHandle *> cfHandles{};
// column families in the DB
static std::vector<std::string> cfNames{"default",
                                        "height_hashes_btc",
                                        "hash_block_btc",
                                        "height_hashes_vbk",
                                        "hash_block_vbk"};

static rocksdb::Status openDB() {
  /// TODO: erase DB before opening otherwise it may fail if scheme was changed

  // prepare column families
  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
  for (std::string cfName : cfNames) {
    column_families.push_back(rocksdb::ColumnFamilyDescriptor(
        cfName, rocksdb::ColumnFamilyOptions()));
  }
  cfHandles.clear();
  rocksdb::Options options;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  rocksdb::Status s = rocksdb::DB::Open(
      options, dbName, column_families, &cfHandles, &dbInstance);
  if (!s.ok()) return s;
  return s;
}

// this class allows to properly close DB before opening it again
class TestStorage : public ::testing::Test {
 protected:
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>>
      cfHandlePtrs;
  // block storage
  BlockRepositoryRocks<StoredBtcBlock> repoBtc;
  BlockRepositoryRocks<StoredVbkBlock> repoVbk;

  void SetUp() {
    rocksdb::Status s = openDB();
    ASSERT_TRUE(s.ok());

    // prepare smart pointers to look after DB cleanup
    dbPtr = std::shared_ptr<rocksdb::DB>(dbInstance);
    cfHandlePtrs = std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>>();
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandles) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
      cfHandlePtrs.push_back(cfHandlePtr);
    }

    repoBtc = BlockRepositoryRocks<StoredBtcBlock>(
        dbPtr,
        cfHandlePtrs[(int)CF_NAMES::HEIGHT_HASHES_BTC],
        cfHandlePtrs[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = BlockRepositoryRocks<StoredVbkBlock>(
        dbPtr,
        cfHandlePtrs[(int)CF_NAMES::HEIGHT_HASHES_VBK],
        cfHandlePtrs[(int)CF_NAMES::HASH_BLOCK_VBK]);
  }

  void TearDown() { }
};

TEST_F(TestStorage, ConfigTest) {
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(defaultBlockBtc, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);
  StoredVbkBlock blockVbk = StoredVbkBlock::fromBlock(defaultBlockVbk);
  bool retVbk = repoVbk.put(blockVbk);
  ASSERT_TRUE(retVbk);
}

TEST_F(TestStorage, PutAndGet) {
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
