#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository_rocks.hpp"
#include "veriblock/storage/stored_btcblock.hpp"
#include "veriblock/storage/stored_vbkblock.hpp"

using namespace VeriBlock;

static const BtcBlock btcBlock1{
    536870912u,
    "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
    1555501858u,
    436279940u,
    (uint32_t)-1695416274};

static const BtcBlock btcBlock2{
    545259520,
    "fc61cc9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000"_unhex,
    "a31508d4b101d0ad11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25"_unhex,
    1553697574,
    388767596,
    (uint32_t)-1351345951};

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
  rocksdb::ColumnFamilyOptions cfOption{};
  for (const std::string &cfName : cfNames) {
    rocksdb::ColumnFamilyDescriptor descriptor(cfName, cfOption);
    column_families.push_back(descriptor);
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
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> cfHandlePtrs;
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

  void TearDown() {}
};

TEST_F(TestStorage, SimplePut) {
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(btcBlock1, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);
  StoredVbkBlock blockVbk = StoredVbkBlock::fromBlock(defaultBlockVbk);
  bool retVbk = repoVbk.put(blockVbk);
  ASSERT_TRUE(retVbk);
}

TEST_F(TestStorage, PutAndGet) {
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(btcBlock1, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);

  StoredBtcBlock readBlock;
  bool readResult = repoBtc.getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  EXPECT_EQ(readBlock.height, blockBtc.height);
  EXPECT_EQ(readBlock.hash, blockBtc.hash);
  EXPECT_EQ(readBlock.hash, btcBlock1.getHash());
  EXPECT_EQ(readBlock.height, 0);
  EXPECT_EQ(readBlock.block.version, btcBlock1.version);
  EXPECT_EQ(readBlock.block.previousBlock, btcBlock1.previousBlock);
  EXPECT_EQ(readBlock.block.timestamp, btcBlock1.timestamp);
  EXPECT_EQ(readBlock.block.bits, btcBlock1.bits);
}

TEST_F(TestStorage, PutAndGetVbk) {
  StoredVbkBlock blockVbk = StoredVbkBlock::fromBlock(defaultBlockVbk);
  bool retVbk = repoVbk.put(blockVbk);
  ASSERT_TRUE(retVbk);

  StoredVbkBlock readBlock;
  bool readResult = repoVbk.getByHash(defaultBlockVbk.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  EXPECT_EQ(readBlock.height, defaultBlockVbk.height);
  EXPECT_EQ(readBlock.hash, defaultBlockVbk.getHash());
  EXPECT_EQ(readBlock.block.getHash(), defaultBlockVbk.getHash());
  EXPECT_EQ(readBlock.block.version, defaultBlockVbk.version);
  EXPECT_EQ(readBlock.block.previousBlock, defaultBlockVbk.previousBlock);
  EXPECT_EQ(readBlock.block.timestamp, defaultBlockVbk.timestamp);
}

TEST_F(TestStorage, GetManyByHash) {
  StoredBtcBlock storedBtcBlock1 = StoredBtcBlock::fromBlock(btcBlock1, 0);
  StoredBtcBlock storedBtcBlock2 = StoredBtcBlock::fromBlock(btcBlock2, 0);
  bool retBtc = repoBtc.put(storedBtcBlock1);
  ASSERT_TRUE(retBtc);
  retBtc = repoBtc.put(storedBtcBlock2);
  ASSERT_TRUE(retBtc);

  std::vector<uint256> hashes{storedBtcBlock1.hash, storedBtcBlock2.hash};
  Slice<const uint256> hashesSlice(hashes.data(), hashes.size());
  std::vector<StoredBtcBlock> out{};
  size_t readResult = repoBtc.getManyByHash(hashesSlice, &out);
  ASSERT_EQ(readResult, 2);

  EXPECT_EQ(out.size(), 2);

  StoredBtcBlock blockOut1 = out[0];
  StoredBtcBlock blockOut2 = out[1];
  if (blockOut1.hash != storedBtcBlock1.hash) {
    blockOut1 = out[1];
    blockOut2 = out[0];
  }

  EXPECT_EQ(blockOut1.hash, storedBtcBlock1.hash);
  EXPECT_EQ(blockOut2.hash, storedBtcBlock2.hash);
}

TEST_F(TestStorage, RemoveByHash) {
  StoredBtcBlock storedBtcBlock1 = StoredBtcBlock::fromBlock(btcBlock1, 0);
  bool retBtc = repoBtc.put(storedBtcBlock1);
  ASSERT_TRUE(retBtc);

  StoredBtcBlock readBlock;
  bool readResult = repoBtc.getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  bool deleteResult = repoBtc.removeByHash(btcBlock1.getHash());
  ASSERT_TRUE(deleteResult);

  readResult = repoBtc.getByHash(btcBlock1.getHash(), &readBlock);
  EXPECT_FALSE(readResult);
}

TEST_F(TestStorage, RemoveNonExisting) {
  uint256 zeroHash{};
  bool deleteResult = repoBtc.removeByHash(zeroHash);
  EXPECT_FALSE(deleteResult);
}

TEST_F(TestStorage, RemovePartially) {
  StoredBtcBlock storedBtcBlock1 = StoredBtcBlock::fromBlock(btcBlock1, 0);
  StoredBtcBlock storedBtcBlock2 = StoredBtcBlock::fromBlock(btcBlock2, 1);
  bool retBtc = repoBtc.put(storedBtcBlock1);
  ASSERT_TRUE(retBtc);
  retBtc = repoBtc.put(storedBtcBlock2);
  ASSERT_TRUE(retBtc);

  StoredBtcBlock readBlock;
  bool readResult = repoBtc.getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);
  readResult = repoBtc.getByHash(btcBlock2.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  size_t deleteResult = repoBtc.removeByHash(btcBlock1.getHash());
  EXPECT_EQ(deleteResult, 1);

  readResult = repoBtc.getByHash(btcBlock1.getHash(), &readBlock);
  EXPECT_FALSE(readResult);

  readResult = repoBtc.getByHash(btcBlock2.getHash(), &readBlock);
  EXPECT_TRUE(readResult);
}
