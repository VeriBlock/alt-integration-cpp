// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/cache/small_lfru_cache.hpp>

using namespace altintegration;
using namespace cache;

TEST(StaticLfuCache, Full) {
  const size_t window = 1;
  setMockTime(100);
  SmallLFRUCache<int, int, 2, window> cache;

  auto v = std::make_shared<int>(5);

  std::shared_ptr<int> value;
  ASSERT_FALSE(cache.get(0, value));

  cache.insert(0, v);
  ASSERT_TRUE(cache.get(0, value));
  ASSERT_FALSE(cache.get(1, value));
  cache.insert(1, v);
  ASSERT_TRUE(cache.get(0, value));
  ASSERT_TRUE(cache.get(1, value));
  cache.insert(2, v);
  ASSERT_TRUE(cache.get(0, value));
  ASSERT_FALSE(cache.get(1, value));
  ASSERT_TRUE(cache.get(2, value));
  ASSERT_EQ(*value, 5);
  cache.insert(3, v);

  ASSERT_TRUE(cache.get(0, value));
  ASSERT_TRUE(cache.get(3, value));

  setMockTime(103);
  // 3 has not been touched since 100
  cache.insert(4, v);

  ASSERT_TRUE(cache.get(0, value));
  ASSERT_FALSE(cache.get(3, value));
  ASSERT_TRUE(cache.get(4, value));

  // 0 has been touched at 103
  setMockTime(105);
  ASSERT_TRUE(cache.get(4, value));
  cache.insert(5, v);
  ASSERT_FALSE(cache.get(0, value));
}
