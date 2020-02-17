#include <gtest/gtest.h>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/storage/block_repository_rocks_setup.hpp"
#include "veriblock/uint.hpp"

using namespace VeriBlock;

enum class TEST_RUNS { INMEM = 0, ROCKS_BTC, ROCKS_VBK };

struct BlockBasic {
  using hash_t = uint256;
  using height_t = int;

  hash_t getHash() const { return hash; }

  hash_t hash{};
  int content = 0;

  std::string toRaw() const {
    WriteStream stream;
    stream.writeLE<height_t>(content);
    stream.write(hash);
    return std::string(reinterpret_cast<const char*>(stream.data().data()),
                       stream.data().size());
  }

  static BlockBasic fromRaw(const std::string& bytes) {
    ReadStream stream(bytes);
    BlockBasic block;
    block.content = stream.readLE<height_t>();
    block.hash = stream.read(sizeof(hash_t));
    return block;
  }

  bool operator==(const BlockBasic& b) const {
    return hash == b.hash && content == b.content;
  }
};

// DB name
static const std::string dbName = "db-test";

class StorageTest : public testing::TestWithParam<TEST_RUNS> {
 public:
  std::shared_ptr<BlockRepository<BlockBasic>> repo;

  StorageTest() : database(dbName) {
    TEST_RUNS repoSelector = GetParam();
    if (repoSelector == TEST_RUNS::INMEM) {
      inmemRepo = std::make_shared<BlockRepositoryInmem<BlockBasic>>();
    } else {
      rocksdb::Status s = database.Open();
      rocksBtcRepo = database.repoBtc;
      rocksVbkRepo = database.repoVbk;
    }

    switch (repoSelector) {
      case TEST_RUNS::INMEM:
        repo = inmemRepo;
        break;
      case TEST_RUNS::ROCKS_BTC:
        repo = rocksBtcRepo;
        break;
      case TEST_RUNS::ROCKS_VBK:
        repo = rocksVbkRepo;
        break;
    }
    repo->clear();
  }

 protected:
  BlockRepositoryRocksInstance<BlockBasic, BlockBasic> database;
  std::shared_ptr<BlockRepositoryInmem<BlockBasic>> inmemRepo;
  std::shared_ptr<BlockRepositoryRocks<BlockBasic>> rocksBtcRepo;
  std::shared_ptr<BlockRepositoryRocks<BlockBasic>> rocksVbkRepo;
};

TEST_P(StorageTest, Basic) {
  using hash_t = BlockBasic::hash_t;
  auto hash1 = hash_t(std::vector<uint8_t>{1});
  auto hash2 = hash_t(std::vector<uint8_t>{2});
  auto hash3 = hash_t(std::vector<uint8_t>{3});
  auto hash4 = hash_t(std::vector<uint8_t>{4});

  BlockBasic b;
  EXPECT_FALSE(repo->put({hash1, 1}));
  EXPECT_TRUE(repo->getByHash(hash1, &b));
  EXPECT_EQ(b.hash, hash1);
  EXPECT_EQ(b.content, 1);

  EXPECT_FALSE(repo->put({hash2, 2}));
  EXPECT_FALSE(repo->put({hash3, 3}));
  EXPECT_TRUE(repo->put({hash1, 5}));

  // block has been overwritten
  EXPECT_TRUE(repo->getByHash(hash1, &b));
  EXPECT_EQ(b.hash, hash1);
  EXPECT_EQ(b.content, 5);

  EXPECT_TRUE(repo->getByHash(hash2, &b));
  EXPECT_EQ(b.getHash(), hash2);

  EXPECT_TRUE(repo->getByHash(hash3, &b));
  EXPECT_EQ(b.getHash(), hash3);

  EXPECT_FALSE(repo->getByHash(hash4, &b));

  std::vector<hash_t> keys{hash1, hash2, hash4};
  std::vector<BlockBasic> blocks;
  size_t size = repo->getManyByHash(keys, &blocks);
  EXPECT_EQ(size, 2);
  EXPECT_EQ(blocks[0], (BlockBasic{hash1, 5}));
  EXPECT_EQ(blocks[1], (BlockBasic{hash2, 2}));

  EXPECT_TRUE(repo->removeByHash(hash1));

  auto hash10 = hash_t(std::vector<uint8_t>{10});
  EXPECT_FALSE(repo->removeByHash(hash10));
}

void checkContents(BlockRepository<BlockBasic>& repo,
                   const std::vector<BlockBasic>& blocks) {
  std::set<BlockBasic::hash_t> a;
  auto c = repo.newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    a.insert(c->key());
  }

  std::vector<BlockBasic::hash_t> v;
  std::set<BlockBasic::hash_t> b;
  for (auto& block : blocks) {
    b.insert(block.hash);
    v.push_back(block.hash);
  }

  std::vector<BlockBasic> read;
  size_t size = repo.getManyByHash(v, &read);
  EXPECT_EQ(size, blocks.size()) << "size is not equal";
  EXPECT_EQ(a, b) << "keys are not equal";
  EXPECT_EQ(read, blocks) << "blocks are not equal";
}

TEST_P(StorageTest, Batch) {
  using hash_t = BlockBasic::hash_t;
  auto hash1 = hash_t(std::vector<uint8_t>{1});
  auto hash2 = hash_t(std::vector<uint8_t>{2});
  auto hash3 = hash_t(std::vector<uint8_t>{3});
  auto hash4 = hash_t(std::vector<uint8_t>{4});
  auto hash5 = hash_t(std::vector<uint8_t>{5});

  EXPECT_FALSE(repo->put({hash1, 1}));
  EXPECT_FALSE(repo->put({hash2, 2}));
  EXPECT_FALSE(repo->put({hash3, 3}));

  checkContents(*repo,
                {
                    {hash1, 1},
                    {hash2, 2},
                    {hash3, 3},
                });

  // commit empty batch does nothing
  auto batch = repo->newBatch();
  batch->commit(*repo);

  checkContents(*repo,
                {
                    {hash1, 1},
                    {hash2, 2},
                    {hash3, 3},
                });

  // commit some changes
  batch->put({hash4, 4});
  batch->put({hash5, 5});
  batch->put({hash1, 9});      // overwrite kv
  batch->removeByHash(hash2);  // remove existing hash
  batch->removeByHash(hash5);  // remove key added in batch

  auto hash100 = hash_t(std::vector<uint8_t>{100});
  batch->removeByHash(hash100);  // remove non-existing key
  batch->commit(*repo);

  checkContents(*repo,
                {
                    {hash1, 9},
                    {hash3, 3},
                    {hash4, 4},
                });
}

TEST_P(StorageTest, Cursor) {
  using hash_t = BlockBasic::hash_t;
  auto hash1 = hash_t(std::vector<uint8_t>{1});
  auto hash2 = hash_t(std::vector<uint8_t>{2});
  auto hash3 = hash_t(std::vector<uint8_t>{3});
  auto hash4 = hash_t(std::vector<uint8_t>{4});

  EXPECT_FALSE(repo->put({hash1, 1}));
  EXPECT_FALSE(repo->put({hash2, 2}));
  EXPECT_FALSE(repo->put({hash3, 3}));
  EXPECT_FALSE(repo->put({hash4, 4}));

  // save order in which blocks appear in DB
  std::vector<BlockBasic> values;
  auto c = repo->newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    values.push_back(c->value());
  }

  c->seekToFirst();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[0].hash);
  EXPECT_EQ(c->value(), values[0]);

  c->seekToLast();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[values.size() - 1].hash);
  EXPECT_EQ(c->value(), values[values.size() - 1]);

  // read in reversed order
  std::vector<BlockBasic> reversedValues;
  for (c->seekToLast(); c->isValid(); c->prev()) {
    reversedValues.push_back(c->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  c->seek(hash1);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (BlockBasic{hash1, 1}));

  // iterate before first element
  EXPECT_NO_FATAL_FAILURE(c->seekToFirst());
  EXPECT_NO_FATAL_FAILURE(c->prev());
  EXPECT_NO_FATAL_FAILURE(c->isValid());
  EXPECT_FALSE(c->isValid());

  // iterate after last element
  EXPECT_NO_FATAL_FAILURE(c->seekToLast());
  EXPECT_NO_FATAL_FAILURE(c->next());
  EXPECT_NO_FATAL_FAILURE(c->isValid());
  EXPECT_FALSE(c->isValid());

  c->seek(hash2);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (BlockBasic{hash2, 2}));

  auto hash100 = hash_t(std::vector<uint8_t>{100});
  c->seek(hash100);  // non-existing key
  EXPECT_FALSE(c->isValid());
  // user is responsible for maintaining cursor validity
  EXPECT_THROW(c->value(), std::out_of_range);
}

static std::vector<TEST_RUNS> testCases = {
    TEST_RUNS::INMEM, TEST_RUNS::ROCKS_BTC, TEST_RUNS::ROCKS_VBK};

INSTANTIATE_TEST_SUITE_P(StorageRegression,
                         StorageTest,
                         testing::ValuesIn(testCases));