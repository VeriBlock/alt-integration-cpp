// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <string>

#include "veriblock/value_sorted_map.hpp"

using namespace altintegration;

struct IntCmp {
  bool operator()(int a, int b) const { return a < b; }
};

TEST(ValueSortedMap, Basic_test) {
  ValueSortedMap<std::string, int, IntCmp> map;

  map.insert("hello", 5);
  map.insert("hello 2", 6);
  map.insert("hello", 10);

  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.empty());

  map.erase("hello 1");
  map.erase("hello");

  EXPECT_EQ(map.size(), 1);
  EXPECT_FALSE(map.empty());

  map.clear();

  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

TEST(ValueSortedMap, find_test) {
  ValueSortedMap<std::string, int, IntCmp> map;

  map.insert("hello", 5);
  map.insert("hello 2", 6);
  map.insert("hello", 10);

  EXPECT_EQ(map.find("hello 3"), map.end());

  auto it = map.find("hello 2");
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, 6);
}