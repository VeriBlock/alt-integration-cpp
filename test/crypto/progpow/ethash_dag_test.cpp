// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>
#include <veriblock/crypto/progpow/ethash.h>

#include <veriblock/crypto/progpow.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/serde.hpp>
#include <veriblock/strutil.hpp>
#include <veriblock/uint.hpp>

#include "ethash_expected_dag.hpp"
#include "progpow_expected_mix_state.hpp"

using namespace altintegration;

TEST(Ethash, CalculateDagNode) {
  uint64_t blockNumber = 1000000;
  std::shared_ptr<ethash_light> light(ethash_light_new(blockNumber),
                                      ethash_light_delete);

  ethash_dag_node_t node;
  ethash_calculate_dag_node(&node, 100, light.get());

  WriteStream w;
  for (int i = 0; i < ETHASH_DAG_NODE_SIZE; i++) {
    w.writeLE<uint32_t>(node.words[i]);
  }

  std::string expected =
      "6ab163e3ddf2535bcb164c366801c3f9ef66145158f5620ab40cb3faf685e31e1c148a24"
      "3b4dbaca9e908e2378cd64dee8cc5ac2a632fd4d2ea4d98ab56cd982";
  std::string actual = HexStr(w.data());

  ASSERT_EQ(actual, expected);
}

// added to access these functions
extern "C" {
uint64_t ethash_get_datasize(uint64_t const block_number);
uint64_t ethash_get_cachesize(uint64_t const block_number);
}

static const int EPOCH_SIZE = 8000;

TEST(Ethash, CacheSize) {
  auto checkRange = [](int base, int expect) {
    for (int i = base * EPOCH_SIZE; i < (base + 1) * EPOCH_SIZE; i++) {
      ASSERT_EQ(ethash_get_cachesize(i), expect);
    }
  };

  // these values are pre-calculated in Nodecore
  // c4b4ed0ea5397ece5d104b53db982e1629be83ce
  checkRange(0, 59113024U);
  checkRange(1, 59243456U);
  checkRange(2, 59375552U);
  checkRange(30, 63045056U);
  checkRange(999, 190054208U);
  checkRange(4095, 595851968U);

  // if this test is built in release, there will be no death
#ifndef NDEBUG
  ASSERT_DEATH(ethash_get_cachesize(4096 * EPOCH_SIZE), "");
#endif
}

TEST(Ethash, DAGSize) {
  auto checkRange = [](int base, long expect) {
    for (int i = base * EPOCH_SIZE; i < (base + 1) * EPOCH_SIZE; i++) {
      ASSERT_EQ(ethash_get_datasize(i), expect);
    }
  };

  // these values are pre-calculated in Nodecore
  // c4b4ed0ea5397ece5d104b53db982e1629be83ce
  checkRange(0, 3783260032ULL);
  checkRange(1, 3791650432ULL);
  checkRange(2, 3800036224ULL);
  checkRange(30, 4034911616ULL);
  checkRange(999, 12163481216ULL);
  checkRange(4095, 38134611584ULL);

  // if this test is built in release, there will be no death
#ifndef NDEBUG
  ASSERT_DEATH(ethash_get_datasize(4096 * EPOCH_SIZE), "");
#endif

  ASSERT_EQ(ethash_get_datasize(1000000ULL), 4831835776ULL);
}

// added to get access to this function, as it is not static and has no
// declaration in header
namespace altintegration::progpow {
#define PROGPOW_LANES 16
#define PROGPOW_REGS 32

std::vector<uint32_t> createDagCache(ethash_light_t light);
void progPowLoop(const uint64_t block_number,
                 const uint32_t loop,
                 uint32_t mix[PROGPOW_LANES][PROGPOW_REGS],
                 const std::vector<uint32_t>& dag,
                 ethash_light_t light);
}  // namespace altintegration::progpow

TEST(Ethash, CreateDagCache) {
  uint64_t blockNumber = 1000000;
  std::shared_ptr<ethash_light> light(ethash_light_new(blockNumber),
                                      ethash_light_delete);

  auto dag = progpow::createDagCache(light.get());

  ASSERT_EQ(dag.size(), 16384);
  ASSERT_EQ(dag, ethash_expected_dag);
}

TEST(ProgPowUtil, Loop) {
  uint64_t blockNumber = 1000000, loop = 0;
  uint32_t mix[PROGPOW_LANES][PROGPOW_REGS];
  for (int i = 0; i < PROGPOW_LANES; i++) {
    for (int j = 0; j < PROGPOW_REGS; j++) {
      mix[i][j] = 0;
    }
  }

  std::shared_ptr<ethash_light> light(ethash_light_new(blockNumber),
                                      ethash_light_delete);
  auto dag = progpow::createDagCache(light.get());
  progpow::progPowLoop(blockNumber, loop, mix, dag, light.get());

  for (int i = 0; i < PROGPOW_LANES; i++) {
    ASSERT_EQ(expected_mix[i],
              (std::vector<uint32_t>{mix[i], mix[i] + PROGPOW_REGS}));
  }
}