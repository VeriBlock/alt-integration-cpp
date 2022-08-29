// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <veriblock/pop/crypto/progpow/cache.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

using namespace altintegration;

TEST(CacheEntry, serde) {
  CacheEntry entry;
  entry.light = progpow::ethash_make_cache(0);
  entry.dag = {1, 2, 3, 4, 5, 6};
  WriteStream w_stream;

  entry.toVbkEncoding(w_stream);

  ReadStream r_stream(w_stream.data());
  CacheEntry deserialized;
  deserialized.light = progpow::ethash_make_cache(10);
  ValidationState state;

  ASSERT_TRUE(DeserializeFromVbkEncoding(r_stream, deserialized, state))
      << state.toString();
  ASSERT_EQ(entry.dag, deserialized.dag);
  ASSERT_EQ(entry.light->cache_size, deserialized.light->cache_size);
  ASSERT_EQ(entry.light->epoch, deserialized.light->epoch);
}
