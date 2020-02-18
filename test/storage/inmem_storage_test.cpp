#include <gtest/gtest.h>

#include <veriblock/storage/block_repository_inmem.hpp>

using namespace VeriBlock;

struct Block {
  using hash_t = int;
  using height_t = int;

  hash_t getHash() const { return hash; }

  int hash = 0;
  int content = 0;

  bool operator==(const Block& b) const {
    return hash == b.hash && content == b.content;
  }
};

struct InmemTest : public ::testing::Test {
  std::shared_ptr<BlockRepository<Block>> repo =
      std::make_shared<BlockRepositoryInmem<Block>>();
};

TEST_F(InmemTest, Basic) {
  Block b;

  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_TRUE(repo->getByHash(1, &b));
  EXPECT_EQ(b.hash, 1);
  EXPECT_EQ(b.content, 1);

  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_TRUE(repo->put({1, 5}));

  // block has been overwritten
  EXPECT_TRUE(repo->getByHash(1, &b));
  EXPECT_EQ(b.hash, 1);
  EXPECT_EQ(b.content, 5);

  EXPECT_TRUE(repo->getByHash(2, &b));
  EXPECT_EQ(b.getHash(), 2);

  EXPECT_TRUE(repo->getByHash(3, &b));
  EXPECT_EQ(b.getHash(), 3);

  EXPECT_FALSE(repo->getByHash(4, &b));

  std::vector<int> keys{1, 2, 4};
  std::vector<Block> blocks;
  size_t size = repo->getManyByHash(keys, &blocks);
  EXPECT_EQ(size, 2);
  EXPECT_EQ(blocks[0], (Block{1, 5}));
  EXPECT_EQ(blocks[1], (Block{2, 2}));

  EXPECT_TRUE(repo->removeByHash(1));
  EXPECT_FALSE(repo->removeByHash(10));
}

void checkContents(BlockRepository<Block>& repo,
                   const std::vector<Block>& blocks) {
  std::set<int> a;
  auto c = repo.newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    a.insert(c->key());
  }

  std::vector<int> v;
  std::set<int> b;
  for (auto& block : blocks) {
    b.insert(block.hash);
    v.push_back(block.hash);
  }

  std::vector<Block> read;
  size_t size = repo.getManyByHash(v, &read);
  EXPECT_EQ(size, blocks.size()) << "size is not equal";
  EXPECT_EQ(a, b) << "keys are not equal";
  EXPECT_EQ(read, blocks) << "blocks are not equal";
}

TEST_F(InmemTest, Batch) {
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
  batch->put({1, 9});         // overwrite kv
  batch->removeByHash(2);     // remove existing hash
  batch->removeByHash(5);     // remove key added in batch
  batch->removeByHash(1000);  // remove non-existing key
  batch->commit(*repo);

  checkContents(*repo,
                {
                    {1, 9},
                    {3, 3},
                    {4, 4},
                });
}

TEST_F(InmemTest, Cursor) {
  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_FALSE(repo->put({4, 4}));

  // save order in which blocks appear in DB
  std::vector<Block> values;
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
  std::vector<Block> reversedValues;
  for (c->seekToLast(); c->isValid(); c->prev()) {
    reversedValues.push_back(c->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  c->seek(1);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (Block{1, 1}));

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

  c->seek(2);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (Block{2, 2}));
  c->seek(1000);  // non-existing key
  EXPECT_FALSE(c->isValid());
  // user is responsible for maintaining cursor validity
  EXPECT_DEATH(c->value(), "");
}
