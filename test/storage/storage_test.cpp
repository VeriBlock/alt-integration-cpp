#include <gtest/gtest.h>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/storage/block_repository_rocks_manager.hpp"
#include "veriblock/uint.hpp"

using namespace VeriBlock;

struct BlockBasic {
  using hash_t = uint256;
  using height_t = int;

  hash_t getHash() const { return std::move(ArithUint256(hashValue)); }

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

template <typename Repo_type>
std::shared_ptr<Repo_type> getRepo();

template <>
std::shared_ptr<BlockRepositoryInmem<BlockBasic>> getRepo() {
  return std::make_shared<BlockRepositoryInmem<BlockBasic>>();
}

template <>
std::shared_ptr<BlockRepositoryRocks<BlockIndex<BtcBlock>>> getRepo() {
  BlockRepositoryRocksManager database(dbName);
  rocksdb::Status s = database.open();
  database.clear();
  return database.repoBtc;
}

template <>
std::shared_ptr<BlockRepositoryRocks<BlockIndex<VbkBlock>>> getRepo() {
  BlockRepositoryRocksManager database(dbName);
  rocksdb::Status s = database.open();
  database.clear();
  return database.repoVbk;
}

template <typename Block_t>
Block_t generateBlock(int a, int b);

template <>
BlockBasic generateBlock(int a, int b) {
  return {a, b};
}

template <>
BlockIndex<BtcBlock> generateBlock(int a, int b) {
  BlockIndex<BtcBlock> block;
  // fill arbitrary fields
  block.height = b;
  block.header.timestamp = a;
  return block;
}

template <>
BlockIndex<VbkBlock> generateBlock(int a, int b) {
  BlockIndex<VbkBlock> block;
  // fill arbitrary fields
  block.height = b;
  block.header.timestamp = a;
  return block;
}

template <typename BlockRepoType>
struct StorageTest : public ::testing::Test {
  using block_t = typename BlockRepoType::stored_block_t;
  using repo_t = BlockRepoType;

  std::shared_ptr<repo_t> repo;

  StorageTest() {
    repo = getRepo<repo_t>();
    repo->clear();
  }
};

TYPED_TEST_SUITE_P(StorageTest);

TYPED_TEST_P(StorageTest, Basic) {
  using block_t = typename Basic::block_t;

  block_t b;
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(1, 1)));
  EXPECT_TRUE(
      this->repo->getByHash(generateBlock<block_t>(1, 1).getHash(), &b));
  EXPECT_EQ(b, generateBlock<block_t>(1, 1));

  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(2, 2)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(3, 3)));
  EXPECT_TRUE(this->repo->put(generateBlock<block_t>(1, 5)));

  // block has been overwritten
  EXPECT_TRUE(
      this->repo->getByHash(generateBlock<block_t>(1, 5).getHash(), &b));
  EXPECT_EQ(b, generateBlock<block_t>(1, 5));

  EXPECT_TRUE(
      this->repo->getByHash(generateBlock<block_t>(2, 1).getHash(), &b));
  EXPECT_EQ(b, generateBlock<block_t>(2, 2));

  EXPECT_TRUE(
      this->repo->getByHash(generateBlock<block_t>(3, 1).getHash(), &b));
  EXPECT_EQ(b, generateBlock<block_t>(3, 3));

  EXPECT_FALSE(
      this->repo->getByHash(generateBlock<block_t>(4, 1).getHash(), &b));

  std::vector<block_t::hash_t> keys{generateBlock<block_t>(1, 1).getHash(),
                                    generateBlock<block_t>(2, 1).getHash(),
                                    generateBlock<block_t>(4, 1).getHash()};
  std::vector<block_t> blocks;
  size_t size = this->repo->getManyByHash(keys, &blocks);
  EXPECT_EQ(size, 2);
  EXPECT_EQ(blocks[0], (generateBlock<block_t>(1, 5)));
  EXPECT_EQ(blocks[1], (generateBlock<block_t>(2, 2)));

  EXPECT_TRUE(this->repo->removeByHash(generateBlock<block_t>(1, 1).getHash()));
  EXPECT_FALSE(
      this->repo->removeByHash(generateBlock<block_t>(10, 1).getHash()));
}

template <typename BlockRepoType, typename BlockType>
void checkContents(BlockRepoType& repo, const std::vector<BlockType>& blocks) {
  std::set<BlockType::hash_t> a;
  auto c = repo.newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    a.insert(c->key());
  }

  std::vector<BlockType::hash_t> v;
  std::set<BlockType::hash_t> b;
  for (auto& block : blocks) {
    b.insert(block.getHash());
    v.push_back(block.getHash());
  }

  std::vector<BlockType> read;
  size_t size = repo.getManyByHash(v, &read);
  EXPECT_EQ(size, blocks.size()) << "size is not equal";
  EXPECT_EQ(a, b) << "keys are not equal";
  EXPECT_EQ(read, blocks) << "blocks are not equal";
}

TYPED_TEST_P(StorageTest, Batch) {
  using block_t = typename Batch::block_t;

  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(1, 1)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(2, 2)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(3, 3)));

  checkContents<Batch::repo_t, Batch::block_t>(*this->repo,
                                               {
                                                   generateBlock<block_t>(1, 1),
                                                   generateBlock<block_t>(2, 2),
                                                   generateBlock<block_t>(3, 3),
                                               });

  // commit empty batch does nothing
  auto batch = this->repo->newBatch();
  batch->commit();

  checkContents<Batch::repo_t, Batch::block_t>(*this->repo,
                                               {
                                                   generateBlock<block_t>(1, 1),
                                                   generateBlock<block_t>(2, 2),
                                                   generateBlock<block_t>(3, 3),
                                               });

  // commit some changes
  batch->put(generateBlock<block_t>(4, 4));
  batch->put(generateBlock<block_t>(5, 5));
  batch->put(generateBlock<block_t>(1, 9));  // overwrite kv
  batch->removeByHash(
      generateBlock<block_t>(2, 1).getHash());  // remove existing hash
  batch->removeByHash(
      generateBlock<block_t>(5, 1).getHash());  // remove key added in batch
  batch->removeByHash(
      generateBlock<block_t>(1000, 1).getHash());  // remove non-existing key
  batch->commit();

  checkContents<Batch::repo_t, Batch::block_t>(*this->repo,
                                               {
                                                   generateBlock<block_t>(1, 9),
                                                   generateBlock<block_t>(3, 3),
                                                   generateBlock<block_t>(4, 4),
                                               });
}

TYPED_TEST_P(StorageTest, Cursor) {
  using block_t = typename Cursor::block_t;

  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(1, 10)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(2, 2)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(3, 3)));
  EXPECT_FALSE(this->repo->put(generateBlock<block_t>(4, 4)));

  // save order in which blocks appear in DB
  std::vector<block_t> values;
  auto c = this->repo->newCursor();
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
  std::vector<block_t> reversedValues;
  for (c->seekToLast(); c->isValid(); c->prev()) {
    reversedValues.push_back(c->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  c->seek(generateBlock<block_t>(1, 10).getHash());
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), generateBlock<block_t>(1, 10));

  // iterate before first element
  c->seekToFirst();
  c->prev();
  EXPECT_FALSE(c->isValid());

  // iterate after last element
  c->seekToLast();
  c->next();
  EXPECT_FALSE(c->isValid());

  c->seek(generateBlock<block_t>(2, 2).getHash());
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), generateBlock<block_t>(2, 2));

  // TODO: fix this. This part fails in case when Seek realisation of the
  // rocksdb finds something and it is not expected.
  //
  // Here it is notes about rocksdb Seek:
  // This is the traditional seek behavior you'd expect. The seek
  // performs on a total ordered key space, positioning the iterator to a key
  // that is greater or equal to the target key you seek.
  /*c->seek(generateBlock<block_t>(1000, 1).getHash());  // non-existing key
  EXPECT_FALSE(c->isValid());
  // user is responsible for maintaining cursor validity
  EXPECT_THROW(c->value(), std::out_of_range);*/
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(StorageTest, Basic, Batch, Cursor);

typedef ::testing::Types<BlockRepositoryInmem<BlockBasic>,
                         BlockRepositoryRocks<BlockIndex<BtcBlock>>,
                         BlockRepositoryRocks<BlockIndex<VbkBlock>>>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(StorageTestSuite, StorageTest, TypesUnderTest);
