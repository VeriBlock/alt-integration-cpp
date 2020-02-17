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

  hash_t getHash() const {
    ArithUint256 hashValue256 = ArithUint256(hashValue);
    return hashValue256;
  }

  int hashValue = 0;
  int content = 0;

  std::string toRaw() const {
    WriteStream stream;
    stream.writeLE<height_t>(content);
    stream.writeLE<int>(hashValue);
    return std::string(reinterpret_cast<const char*>(stream.data().data()),
                       stream.data().size());
  }

  static BlockBasic fromRaw(const std::string& bytes) {
    ReadStream stream(bytes);
    BlockBasic block;
    block.content = stream.readLE<height_t>();
    block.hashValue = stream.readLE<int>();
    return block;
  }

  bool operator==(const BlockBasic& b) const {
    return hashValue == b.hashValue && content == b.content;
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
  BlockBasic b;
  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_TRUE(repo->getByHash(BlockBasic{1, 1}.getHash(), &b));
  EXPECT_EQ(b.hashValue, 1);
  EXPECT_EQ(b.content, 1);

  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_TRUE(repo->put({1, 5}));

  // block has been overwritten
  EXPECT_TRUE(repo->getByHash(BlockBasic{1, 1}.getHash(), &b));
  EXPECT_EQ(b.hashValue, 1);
  EXPECT_EQ(b.content, 5);

  EXPECT_TRUE(repo->getByHash(BlockBasic{2, 1}.getHash(), &b));
  EXPECT_EQ(b.hashValue, 2);

  EXPECT_TRUE(repo->getByHash(BlockBasic{3, 1}.getHash(), &b));
  EXPECT_EQ(b.hashValue, 3);

  EXPECT_FALSE(repo->getByHash(BlockBasic{4, 1}.getHash(), &b));

  std::vector<BlockBasic::hash_t> keys{BlockBasic{1, 1}.getHash(),
                           BlockBasic{2, 1}.getHash(),
                           BlockBasic{4, 1}.getHash()};
  std::vector<BlockBasic> blocks;
  size_t size = repo->getManyByHash(keys, &blocks);
  EXPECT_EQ(size, 2);
  EXPECT_EQ(blocks[0], (BlockBasic{1, 5}));
  EXPECT_EQ(blocks[1], (BlockBasic{2, 2}));

  EXPECT_TRUE(repo->removeByHash(BlockBasic{1, 1}.getHash()));
  EXPECT_FALSE(repo->removeByHash(BlockBasic{10, 1}.getHash()));
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
    b.insert(block.getHash());
    v.push_back(block.getHash());
  }

  std::vector<BlockBasic> read;
  size_t size = repo.getManyByHash(v, &read);
  EXPECT_EQ(size, blocks.size()) << "size is not equal";
  EXPECT_EQ(a, b) << "keys are not equal";
  EXPECT_EQ(read, blocks) << "blocks are not equal";
}

TEST_P(StorageTest, Batch) {
  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));

  checkContents(*repo,
                {
                    {1, 1},
                    {2, 2},
                    {3, 3},
                });

  // commit empty batch does nothing
  auto batch = repo->newBatch();
  batch->commit(*repo);

  checkContents(*repo,
                {
                    {1, 1},
                    {2, 2},
                    {3, 3},
                });

  // commit some changes
  batch->put({4, 4});
  batch->put({5, 5});
  batch->put({1, 9});      // overwrite kv
  batch->removeByHash(BlockBasic{2, 1}.getHash());  // remove existing hash
  batch->removeByHash(BlockBasic{5, 1}.getHash());  // remove key added in batch
  batch->removeByHash(BlockBasic{1000, 1}.getHash());  // remove non-existing key
  batch->commit(*repo);

  checkContents(*repo,
                {
                    {1, 9},
                    {3, 3},
                    {4, 4},
                });
}

TEST_P(StorageTest, Cursor) {
  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_FALSE(repo->put({4, 4}));

  // save order in which blocks appear in DB
  std::vector<BlockBasic> values;
  auto c = repo->newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    values.push_back(c->value());
  }

  c->seekToFirst();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[0].getHash());
  EXPECT_EQ(c->value(), values[0]);

  c->seekToLast();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[values.size() - 1].getHash());
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
  c->seek(BlockBasic{1, 1}.getHash());
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (BlockBasic{1, 1}));

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

  c->seek(BlockBasic{2, 2}.getHash());
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (BlockBasic{2, 2}));

  c->seek(BlockBasic{1000, 1}.getHash());  // non-existing key
  EXPECT_FALSE(c->isValid());
  // user is responsible for maintaining cursor validity
  EXPECT_THROW(c->value(), std::out_of_range);
}

static std::vector<TEST_RUNS> testCases = {
    TEST_RUNS::INMEM, TEST_RUNS::ROCKS_BTC, TEST_RUNS::ROCKS_VBK};

INSTANTIATE_TEST_SUITE_P(StorageRegression,
                         StorageTest,
                         testing::ValuesIn(testCases));