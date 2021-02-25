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

  it = map.find("hello");
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, 10);
}

TEST(ValueSortedMap, erase_test) {
  ValueSortedMap<std::string, int, IntCmp> map;

  map.insert("key 1", 5);
  map.insert("key 2", 6);
  map.insert("key 3", 10);
  map.insert("key 4", 5);

  EXPECT_EQ(map.size(), 4);
  EXPECT_FALSE(map.empty());

  EXPECT_NE(map.find("key 1"), map.end());
  EXPECT_NE(map.find("key 2"), map.end());
  EXPECT_NE(map.find("key 3"), map.end());
  EXPECT_NE(map.find("key 4"), map.end());

  map.erase("key 4");

  EXPECT_EQ(map.size(), 3);
  EXPECT_FALSE(map.empty());
}

TEST(ValueSortedMap, value_sort_test) {
  IntCmp cmp;
  ValueSortedMap<std::string, int, IntCmp> map(cmp);
  using pair_t = typename ValueSortedMap<std::string, int, IntCmp>::pair_t;

  map.insert("key 1", 4);
  map.insert("key 4", 2);
  map.insert("key 5", 6);
  map.insert("key 6", 1);
  map.insert("key 7", 12);
  map.insert("key 8", 4);
  map.insert("key 9", 6);

  auto it = map.begin();

  EXPECT_EQ(*(it++), pair_t("key 6", 1));
  EXPECT_EQ(*(it++), pair_t("key 4", 2));
  EXPECT_EQ(*(it++), pair_t("key 1", 4));
  EXPECT_EQ(*(it++), pair_t("key 8", 4));
  EXPECT_EQ(*(it++), pair_t("key 5", 6));
  EXPECT_EQ(*(it++), pair_t("key 9", 6));
  EXPECT_EQ(*(it++), pair_t("key 7", 12));
}