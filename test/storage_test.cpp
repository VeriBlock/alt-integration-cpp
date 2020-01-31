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

TEST(Storage, configTest) {
  rocksdb::DB *db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status s = rocksdb::DB::Open(options, "db-test", &db);
  ASSERT_TRUE(s.ok());

  auto dbPtr = std::shared_ptr<rocksdb::DB>(db);
  BlockRepositoryRocks<StoredBtcBlock> repoBtc(dbPtr);
  StoredBtcBlock blockBtc = StoredBtcBlock::fromBlock(defaultBlockBtc, 0);
  bool retBtc = repoBtc.put(blockBtc);
  ASSERT_TRUE(retBtc);

  BlockRepositoryRocks<StoredVbkBlock> repoVbk(dbPtr);
  StoredVbkBlock blockVbk = StoredVbkBlock::fromBlock(defaultBlockVbk);
  bool retVbk = repoVbk.put(blockVbk);
  ASSERT_TRUE(retVbk);
}
