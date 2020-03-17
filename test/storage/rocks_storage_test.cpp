#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/repository_rocks_manager.hpp"

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

// DB name
static const std::string dbName = "db-rocks_storage_test";

class TestStorage : public ::testing::Test {
 protected:
  RepositoryRocksManager database = RepositoryRocksManager(dbName);
  std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> repoBtc;
  std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> repoVbk;

  void SetUp() {
    rocksdb::Status s = database.open();
    ASSERT_TRUE(s.ok()) << s.ToString();
    s = database.clear();
    ASSERT_TRUE(s.ok()) << s.ToString();

    repoBtc = database.getBtcRepo();
    repoVbk = database.getVbkRepo();

    repoBtc->clear();
    repoVbk->clear();
  }
};

TEST_F(TestStorage, SimplePut) {
  BlockIndex<BtcBlock> blockBtc;
  blockBtc.header = btcBlock1;
  blockBtc.height = 0;
  bool retBtc = repoBtc->put(blockBtc);
  EXPECT_FALSE(retBtc);
  BlockIndex<VbkBlock> blockVbk;
  blockVbk.header = defaultBlockVbk;
  blockVbk.height = defaultBlockVbk.height;
  bool retVbk = repoVbk->put(blockVbk);
  EXPECT_FALSE(retVbk);
}

TEST_F(TestStorage, PutAndGet) {
  BlockIndex<BtcBlock> blockBtc;
  blockBtc.header = btcBlock1;
  blockBtc.height = 0;

  bool retBtc = repoBtc->put(blockBtc);
  EXPECT_FALSE(retBtc);

  BlockIndex<BtcBlock> readBlock;
  bool readResult = repoBtc->getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  EXPECT_EQ(readBlock.height, blockBtc.height);
  EXPECT_EQ(readBlock.getHash(), blockBtc.getHash());
  EXPECT_EQ(readBlock.getHash(), btcBlock1.getHash());
  EXPECT_EQ(readBlock.height, 0);
  EXPECT_EQ(readBlock.header.version, btcBlock1.version);
  EXPECT_EQ(readBlock.header.previousBlock, btcBlock1.previousBlock);
  EXPECT_EQ(readBlock.header.timestamp, btcBlock1.timestamp);
  EXPECT_EQ(readBlock.header.bits, btcBlock1.bits);
}

TEST_F(TestStorage, PutAndGetVbk) {
  BlockIndex<VbkBlock> blockVbk;
  blockVbk.header = defaultBlockVbk;
  blockVbk.height = defaultBlockVbk.height;
  bool retVbk = repoVbk->put(blockVbk);
  EXPECT_FALSE(retVbk);

  BlockIndex<VbkBlock> readBlock;
  bool readResult = repoVbk->getByHash(defaultBlockVbk.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  EXPECT_EQ(readBlock.height, defaultBlockVbk.height);
  EXPECT_EQ(readBlock.getHash(), defaultBlockVbk.getHash());
  EXPECT_EQ(readBlock.header.getHash(), defaultBlockVbk.getHash());
  EXPECT_EQ(readBlock.header.version, defaultBlockVbk.version);
  EXPECT_EQ(readBlock.header.previousBlock, defaultBlockVbk.previousBlock);
  EXPECT_EQ(readBlock.header.timestamp, defaultBlockVbk.timestamp);
}

TEST_F(TestStorage, GetManyByHash) {
  BlockIndex<BtcBlock> storedBtcBlock1;
  storedBtcBlock1.header = btcBlock1;
  storedBtcBlock1.height = 0;

  BlockIndex<BtcBlock> storedBtcBlock2;
  storedBtcBlock2.header = btcBlock2;
  storedBtcBlock2.height = 0;
  bool retBtc = repoBtc->put(storedBtcBlock1);
  EXPECT_FALSE(retBtc);
  retBtc = repoBtc->put(storedBtcBlock2);
  EXPECT_FALSE(retBtc);

  std::vector<uint256> hashes{storedBtcBlock1.getHash(),
                              storedBtcBlock2.getHash()};
  Slice<const uint256> hashesSlice(hashes.data(), hashes.size());
  std::vector<BlockIndex<BtcBlock>> out{};
  size_t readResult = repoBtc->getManyByHash(hashesSlice, &out);
  ASSERT_EQ(readResult, 2);

  EXPECT_EQ(out.size(), 2);

  BlockIndex<BtcBlock> blockOut1 = out[0];
  BlockIndex<BtcBlock> blockOut2 = out[1];
  if (blockOut1.getHash() != storedBtcBlock1.getHash()) {
    blockOut1 = out[1];
    blockOut2 = out[0];
  }

  EXPECT_EQ(blockOut1.getHash(), storedBtcBlock1.getHash());
  EXPECT_EQ(blockOut2.getHash(), storedBtcBlock2.getHash());
}

TEST_F(TestStorage, RemoveByHash) {
  BlockIndex<BtcBlock> storedBtcBlock1;
  storedBtcBlock1.header = btcBlock1;
  storedBtcBlock1.height = 0;
  bool retBtc = repoBtc->put(storedBtcBlock1);
  EXPECT_FALSE(retBtc);

  BlockIndex<BtcBlock> readBlock;
  bool readResult = repoBtc->getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  bool deleteResult = repoBtc->removeByHash(btcBlock1.getHash());
  ASSERT_TRUE(deleteResult);

  readResult = repoBtc->getByHash(btcBlock1.getHash(), &readBlock);
  EXPECT_FALSE(readResult);
}

TEST_F(TestStorage, RemoveNonExisting) {
  uint256 zeroHash{};
  bool deleteResult = repoBtc->removeByHash(zeroHash);
  EXPECT_FALSE(deleteResult);
}

TEST_F(TestStorage, RemovePartially) {
  BlockIndex<BtcBlock> storedBtcBlock1;
  storedBtcBlock1.header = btcBlock1;
  storedBtcBlock1.height = 0;
  BlockIndex<BtcBlock> storedBtcBlock2;
  storedBtcBlock2.header = btcBlock2;
  storedBtcBlock2.height = 1;

  bool retBtc = repoBtc->put(storedBtcBlock1);
  EXPECT_FALSE(retBtc);
  retBtc = repoBtc->put(storedBtcBlock2);
  EXPECT_FALSE(retBtc);

  BlockIndex<BtcBlock> readBlock;
  bool readResult = repoBtc->getByHash(btcBlock1.getHash(), &readBlock);
  ASSERT_TRUE(readResult);
  readResult = repoBtc->getByHash(btcBlock2.getHash(), &readBlock);
  ASSERT_TRUE(readResult);

  size_t deleteResult = repoBtc->removeByHash(btcBlock1.getHash());
  EXPECT_EQ(deleteResult, 1);

  readResult = repoBtc->getByHash(btcBlock1.getHash(), &readBlock);
  EXPECT_FALSE(readResult);

  readResult = repoBtc->getByHash(btcBlock2.getHash(), &readBlock);
  EXPECT_TRUE(readResult);
}
