// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>
#include <veriblock/crypto/progpow/ethash.h>
#include <veriblock/third_party/sha256.h>

#include <veriblock/uint.hpp>

#include "ethash_expected_cache.hpp"

using namespace altintegration;

TEST(Ethash, CreateCache) {
  // create cache for block number = 1
  const uint64_t blockNumber = 1;
  ethash_light_t light = ethash_light_new(1);
  uint8_t* b = static_cast<uint8_t*>(light->cache);
  auto first1000ints = HexStr(b, b + 4 * 1000);

  // 14778256 uint32_ts
  ASSERT_EQ(light->cache_size, 14778256 * 4);
  ASSERT_EQ(light->block_number, blockNumber);
  ASSERT_EQ(first1000ints, ethash_expected_cache);
}