#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/storage/block_repository_rocks.hpp"
#include "veriblock/entities/btcblock.hpp"

using namespace VeriBlock;

static const BtcBlock defaultBlock{
    536870912u,
    "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
    1555501858u,
    436279940u,
    (uint32_t)-1695416274};

static const auto zeroHash =
    "0000000000000000000000000000000000000000000000000000000000000000"_unhex;

struct StoredBtcBlock {
  using hash_t = uint256;
  using height_t = uint32_t;

  hash_t hash{};
  height_t height{};
  BtcBlock header{};
};

TEST(Storage, configTest) {
  rocksdb::DB *db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status s = rocksdb::DB::Open(options, "db-test", &db);
  ASSERT_TRUE(s.ok());

  auto dbPtr = std::shared_ptr<rocksdb::DB>(db);
  BlockRepositoryRocks<StoredBtcBlock> repo(dbPtr);
  StoredBtcBlock block{zeroHash, 0, defaultBlock};
  bool ret = repo.put(block);
  ASSERT_TRUE(ret);
}
