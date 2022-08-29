#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <set>
#include <veriblock/pop/entities/atv.hpp"
#include <veriblock/pop/entities/payloads.hpp"
#include <veriblock/pop/entities/vtb.hpp"
#include <veriblock/pop/storage/payloads_repository.hpp"
#include <veriblock/pop/storage/payloads_repository_inmem.hpp"
#include <veriblock/pop/storage/payloads_repository_rocks.hpp"
#include <veriblock/pop/storage/repository_rocks_manager.hpp"

#include "util/literals.hpp"
#include "util/test_utils.hpp"

using namespace altintegration;

// DB name
static const std::string dbName = "db-test";

template <typename Repo_type>
std::shared_ptr<Repo_type> getRepo();

template <typename PayloadsType>
PayloadsType generatePayloads();

template <>
std::shared_ptr<PayloadsRepositoryInmem<AltPayloads>> getRepo() {
  return std::make_shared<PayloadsRepositoryInmem<AltPayloads>>();
}

template <>
std::shared_ptr<PayloadsRepositoryInmem<VTB>> getRepo() {
  return std::make_shared<PayloadsRepositoryInmem<VTB>>();
}

template <>
std::shared_ptr<PayloadsRepositoryRocks<AltPayloads>> getRepo() {
  RepositoryRocksManager database(dbName);
  rocksdb::Status s = database.open();
  database.clear();
  return database.getAltPayloadsRepo();
}

template <>
std::shared_ptr<PayloadsRepositoryRocks<VTB>> getRepo() {
  RepositoryRocksManager database(dbName);
  rocksdb::Status s = database.open();
  database.clear();
  return database.getVbkPayloadsRepo();
}

template <>
AltPayloads generatePayloads() {
  AltPayloads p;
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

template <>
VTB generatePayloads() {
  std::vector<uint8_t> bytes = ParseHex(defaultVtbEncoded);
  auto stream = ReadStream(bytes);
  VTB vtb{VTB::fromVbkEncoding(stream)};
  vtb.transaction.bitcoinTransaction.tx = generateRandomBytesVector(100);
  return vtb;
}

template <typename PayloadsRepoType>
struct PayloadsRepoTest : public ::testing::Test {
  using payloads_t = typename PayloadsRepoType::stored_payloads_t;
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

  using payloads_t = typename Basic::payloads_t;

  payloads_t p1 = generatePayloads<payloads_t>();
  payloads_t p2 = generatePayloads<payloads_t>();
  p1.valid = false;
  p2.valid = false;
  this->repo->put(p1);
  this->repo->put(p2);

  payloads_t stored_p;
  EXPECT_TRUE(this->repo->get(p1.getId(), &stored_p));
  EXPECT_EQ(stored_p, p1);
  EXPECT_EQ(stored_p.valid, p1.valid);
  EXPECT_TRUE(this->repo->get(p2.getId(), &stored_p));
  EXPECT_EQ(stored_p.valid, p2.valid);

  this->repo->removeByHash(p2.getId());

  EXPECT_TRUE(this->repo->get(p1.getId(), &stored_p));
  EXPECT_EQ(stored_p, p1);
  EXPECT_FALSE(this->repo->get(p2.getId(), &stored_p));

  this->repo->put(p1);
  this->repo->put(p2);

  payloads_t p3 = generatePayloads<payloads_t>();
  payloads_t p4 = generatePayloads<payloads_t>();

  this->repo->put(p3);
  this->repo->put(p4);

  EXPECT_TRUE(this->repo->get(p1.getId(), &stored_p));
  EXPECT_EQ(stored_p, p1);
  EXPECT_TRUE(this->repo->get(p2.getId(), &stored_p));
  EXPECT_EQ(stored_p, p2);
  EXPECT_TRUE(this->repo->get(p3.getId(), &stored_p));
  EXPECT_EQ(stored_p, p3);
  EXPECT_TRUE(this->repo->get(p4.getId(), &stored_p));
  EXPECT_EQ(stored_p, p4);

  std::vector<typename payloads_t::id_t> ids = {
      p1.getId(), p2.getId(), p3.getId(), p4.getId()};

  std::vector<payloads_t> stored_ps;
  size_t num = this->repo->get(ids, &stored_ps);
  EXPECT_EQ(num, ids.size());

  for (size_t i = 0; i < ids.size(); ++i) {
    EXPECT_EQ(ids[i], stored_ps[i].getId());
  }

  ids = {p3.getId(), p4.getId(), p1.getId(), p2.getId()};

  stored_ps.clear();
  num = this->repo->get(ids, &stored_ps);
  EXPECT_EQ(num, ids.size());

  for (size_t i = 0; i < ids.size(); ++i) {
    EXPECT_EQ(ids[i], stored_ps[i].getId());
  }

  this->repo->removeByHash(p2.getId());

  ids = {p1.getId(), p2.getId(), p3.getId(), p4.getId()};
  stored_ps.clear();
  num = this->repo->get(ids, &stored_ps);
  EXPECT_EQ(num, ids.size() - 1);

  for (size_t i = 0, j = 0; i < ids.size(); ++i) {
    if (ids[i] != p2.getId()) {
      EXPECT_EQ(ids[i], stored_ps[j++].getId());
    }
  }
}

TYPED_TEST_P(PayloadsRepoTest, Cursor) {
  srand(0);

  using payloads_t = typename Cursor::payloads_t;

  payloads_t p1 = generatePayloads<payloads_t>();
  payloads_t p2 = generatePayloads<payloads_t>();
  this->repo->put(p1);
  this->repo->put(p2);

  payloads_t p3 = generatePayloads<payloads_t>();
  payloads_t p4 = generatePayloads<payloads_t>();
  this->repo->put(p3);
  this->repo->put(p4);

  payloads_t p5 = generatePayloads<payloads_t>();
  payloads_t p6 = generatePayloads<payloads_t>();
  this->repo->put(p5);
  this->repo->put(p6);

  std::vector<payloads_t> values;
  auto cursor = this->repo->newCursor();

  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    values.push_back(cursor->value());
  }

  cursor->seekToFirst();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_EQ(cursor->key(), values[0].getId());
  EXPECT_EQ(cursor->value(), values[0]);

  cursor->seekToLast();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_EQ(cursor->key(), values[values.size() - 1].getId());
  EXPECT_EQ(cursor->value(), values[values.size() - 1]);

  // read in reversed order
  std::vector<payloads_t> reversedValues;
  for (cursor->seekToLast(); cursor->isValid(); cursor->prev()) {
    reversedValues.push_back(cursor->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  cursor->seek(p1.getId());
  EXPECT_TRUE(cursor->isValid());  // key found
  EXPECT_EQ(cursor->key(), p1.getId());
  EXPECT_EQ(cursor->value(), p1);

  // find some values
  cursor->seek(p2.getId());
  EXPECT_TRUE(cursor->isValid());  // key found
  EXPECT_EQ(cursor->key(), p2.getId());
  EXPECT_EQ(cursor->value(), p2);

  // find some values
  cursor->seek(p3.getId());
  EXPECT_TRUE(cursor->isValid());  // key found
  EXPECT_EQ(cursor->key(), p3.getId());
  EXPECT_EQ(cursor->value(), p3);

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

  using payloads_t = typename Batch::payloads_t;

  auto batch = this->repo->newBatch();

  std::vector<payloads_t> payloads;

  payloads_t p1 = generatePayloads<payloads_t>();
  payloads_t p2 = generatePayloads<payloads_t>();
  payloads_t p3 = generatePayloads<payloads_t>();
  payloads_t p4 = generatePayloads<payloads_t>();
  payloads_t p5 = generatePayloads<payloads_t>();
  payloads_t p6 = generatePayloads<payloads_t>();

  batch->put(p1);
  batch->put(p2);

  EXPECT_FALSE(this->repo->get(p1.getId(), nullptr));

  batch->commit();

  payloads_t stored_value;
  EXPECT_TRUE(this->repo->get(p1.getId(), &stored_value));
  EXPECT_EQ(stored_value, p1);
  EXPECT_TRUE(this->repo->get(p2.getId(), &stored_value));
  EXPECT_EQ(stored_value, p2);

  batch->put(p3);
  batch->put(p4);

  batch->put(p5);
  batch->put(p6);

  batch->removeByHash(p2.getId());

  batch->commit();

  EXPECT_TRUE(this->repo->get(p1.getId(), &stored_value));
  EXPECT_EQ(stored_value, p1);
  EXPECT_TRUE(this->repo->get(p3.getId(), &stored_value));
  EXPECT_EQ(stored_value, p3);
  EXPECT_TRUE(this->repo->get(p4.getId(), &stored_value));
  EXPECT_EQ(stored_value, p4);
  EXPECT_TRUE(this->repo->get(p5.getId(), &stored_value));
  EXPECT_EQ(stored_value, p5);
  EXPECT_TRUE(this->repo->get(p6.getId(), &stored_value));
  EXPECT_EQ(stored_value, p6);

  EXPECT_FALSE(this->repo->get(p2.getId(), &stored_value));
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(PayloadsRepoTest, Basic, Cursor, Batch);

typedef ::testing::Types<PayloadsRepositoryInmem<AltPayloads>,
                         PayloadsRepositoryRocks<AltPayloads>,
                         PayloadsRepositoryInmem<VTB>,
                         PayloadsRepositoryRocks<VTB>>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(PayloadsRepoTestSuite,
                               PayloadsRepoTest,
                               TypesUnderTest);
