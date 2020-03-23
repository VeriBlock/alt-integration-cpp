#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <set>

#include "util/literals.hpp"
#include "util/test_utils.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/storage/payloads_repository.hpp"
#include "veriblock/storage/payloads_repository_inmem.hpp"
#include "veriblock/storage/payloads_repository_rocks.hpp"
#include "veriblock/storage/repository_rocks_manager.hpp"

using namespace altintegration;

// DB name
static const std::string dbName = "db-test";

template <typename Repo_type>
std::shared_ptr<Repo_type> getRepo();

template <>
std::shared_ptr<PayloadsRepositoryInmem<AltBlock, Payloads>> getRepo() {
  return std::make_shared<PayloadsRepositoryInmem<AltBlock, Payloads>>();
}

template <>
std::shared_ptr<PayloadsRepositoryRocks<AltBlock, Payloads>> getRepo() {
  RepositoryRocksManager database(dbName);
  rocksdb::Status s = database.open();
  database.clear();
  return database.getPayloadsRepo();
}

Payloads generatePayloads() {
  Payloads p;
  p.alt.containing = {generateRandomBytesVector(32),
                      generateRandomBytesVector(32),
                      (uint32_t)(std::rand() % 1000 + 1),
                      (int32_t)(std::rand() % 1000 + 1)};

  p.alt.endorsed = {generateRandomBytesVector(32),
                    generateRandomBytesVector(32),
                    (uint32_t)(std::rand() % 1000 + 1),
                    (int32_t)(std::rand() % 1000 + 1)};

  std::vector<uint8_t> bytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(bytes);

  p.alt.atv = ATV::fromVbkEncoding(stream);

  bytes = ParseHex(defaultVtbEncoded);
  stream = ReadStream(bytes);

  p.vtbs.push_back(VTB::fromVbkEncoding(stream));

  return p;
}

AltBlock generateBlock() {
  AltBlock b{generateRandomBytesVector(32),
             generateRandomBytesVector(32),
             (uint32_t)(std::rand() % 1000 + 1),
             (int32_t)(std::rand() % 1000 + 1)};
  return b;
}

template <typename PayloadsRepoType>
struct PayloadsRepoTest : public ::testing::Test {
  using payloads_t = typename PayloadsRepoType::stored_payloads_t;
  using block_t = typename PayloadsRepoType::block_t;
  using stored_payloads_container_t =
      typename PayloadsRepoType::stored_payloads_container_t;
  using repo_t = PayloadsRepoType;

  std::shared_ptr<repo_t> repo;

  PayloadsRepoTest() {
    repo = getRepo<repo_t>();
    repo->clear();
  }
};

TYPED_TEST_SUITE_P(PayloadsRepoTest);

TYPED_TEST_P(PayloadsRepoTest, Basic) {
  srand(0);

  using block_t = typename Basic::block_t;
  using payloads_t = typename Basic::payloads_t;

  block_t b1 = generateBlock();
  payloads_t p1 = generatePayloads();
  payloads_t p2 = generatePayloads();
  this->repo->put(b1.hash, p1);
  this->repo->put(b1.hash, p2);

  std::vector<payloads_t> payloads = this->repo->get(b1.hash);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) !=
              payloads.end());

  this->repo->removeByHash(b1.hash);

  payloads = this->repo->get(b1.hash);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) ==
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) ==
              payloads.end());

  this->repo->put(b1.hash, p1);
  this->repo->put(b1.hash, p2);

  block_t b2 = generateBlock();
  payloads_t p3 = generatePayloads();
  payloads_t p4 = generatePayloads();

  this->repo->put(b2.hash, p3);
  this->repo->put(b2.hash, p4);

  payloads = this->repo->get(b1.hash);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p3) ==
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p4) ==
              payloads.end());

  payloads = this->repo->get(b2.hash);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) ==
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) ==
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p3) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p4) !=
              payloads.end());
}

TYPED_TEST_P(PayloadsRepoTest, Cursor) {
  srand(0);

  using block_t = typename Cursor::block_t;
  using payloads_t = typename Cursor::payloads_t;
  using stored_payloads_container_t =
      typename Cursor::stored_payloads_container_t;

  block_t b1 = generateBlock();
  payloads_t p1 = generatePayloads();
  payloads_t p2 = generatePayloads();
  this->repo->put(b1.hash, p1);
  this->repo->put(b1.hash, p2);

  block_t b2 = generateBlock();
  payloads_t p3 = generatePayloads();
  payloads_t p4 = generatePayloads();
  this->repo->put(b2.hash, p3);
  this->repo->put(b2.hash, p4);

  block_t b3 = generateBlock();
  payloads_t p5 = generatePayloads();
  payloads_t p6 = generatePayloads();
  this->repo->put(b3.hash, p5);
  this->repo->put(b3.hash, p6);

  std::vector<stored_payloads_container_t> values;
  auto cursor = this->repo->newCursor();

  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    values.push_back(cursor->value());
  }

  cursor->seekToFirst();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_TRUE((cursor->key() == b1.hash || cursor->key() == b2.hash ||
               cursor->key() == b3.hash));
  EXPECT_EQ(cursor->value(), values[0]);

  cursor->seekToLast();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_TRUE((cursor->key() == b1.hash || cursor->key() == b2.hash ||
               cursor->key() == b3.hash));
  EXPECT_EQ(cursor->value(), values[values.size() - 1]);

  // read in reversed order
  std::vector<stored_payloads_container_t> reversedValues;
  for (cursor->seekToLast(); cursor->isValid(); cursor->prev()) {
    reversedValues.push_back(cursor->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  stored_payloads_container_t cont;

  // find some values
  cursor->seek(b1.hash);
  EXPECT_TRUE(cursor->isValid());  // key found
  cont = cursor->value();
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p1) != cont.end());
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p2) != cont.end());

  // find some values
  cursor->seek(b2.hash);
  EXPECT_TRUE(cursor->isValid());  // key found
  cont = cursor->value();
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p3) != cont.end());
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p4) != cont.end());

  // find some values
  cursor->seek(b3.hash);
  EXPECT_TRUE(cursor->isValid());  // key found
  cont = cursor->value();
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p5) != cont.end());
  EXPECT_TRUE(std::find(cont.begin(), cont.end(), p6) != cont.end());

  // iterate before first element
  cursor->seekToFirst();
  cursor->prev();
  EXPECT_FALSE(cursor->isValid());

  // iterate after last element
  cursor->seekToLast();
  cursor->next();
  EXPECT_FALSE(cursor->isValid());
}

TYPED_TEST_P(PayloadsRepoTest, Batch) {
  srand(0);

  using block_t = typename Batch::block_t;
  using payloads_t = typename Batch::payloads_t;

  auto batch = this->repo->newBatch();

  std::vector<payloads_t> payloads;

  block_t b1 = generateBlock();
  payloads_t p1 = generatePayloads();
  payloads_t p2 = generatePayloads();

  block_t b2 = generateBlock();
  payloads_t p3 = generatePayloads();
  payloads_t p4 = generatePayloads();

  block_t b3 = generateBlock();
  payloads_t p5 = generatePayloads();
  payloads_t p6 = generatePayloads();

  batch->put(b1.hash, p1);
  batch->put(b1.hash, p2);

  payloads = this->repo->get(b1.hash);
  EXPECT_EQ(payloads.size(), 0);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) ==
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) ==
              payloads.end());

  batch->commit();

  payloads = this->repo->get(b1.hash);
  EXPECT_EQ(payloads.size(), 2);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) !=
              payloads.end());

  batch->put(b1.hash, p1);
  batch->put(b1.hash, p2);

  batch->put(b2.hash, p3);
  batch->put(b2.hash, p4);

  batch->put(b3.hash, p5);
  batch->put(b3.hash, p6);

  batch->removeByHash(b2.hash);

  batch->commit();

  payloads = this->repo->get(b1.hash);
  EXPECT_EQ(payloads.size(), 2);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p1) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p2) !=
              payloads.end());

  payloads = this->repo->get(b2.hash);
  EXPECT_EQ(payloads.size(), 0);

  payloads = this->repo->get(b3.hash);
  EXPECT_EQ(payloads.size(), 2);
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p5) !=
              payloads.end());
  EXPECT_TRUE(std::find(payloads.begin(), payloads.end(), p6) !=
              payloads.end());
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(PayloadsRepoTest, Basic, Cursor, Batch);

typedef ::testing::Types<PayloadsRepositoryInmem<AltBlock, Payloads>,
                         PayloadsRepositoryRocks<AltBlock, Payloads>>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(PayloadsRepoTestSuite,
                               PayloadsRepoTest,
                               TypesUnderTest);
