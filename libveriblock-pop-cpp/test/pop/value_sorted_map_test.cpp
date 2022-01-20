// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <string>
#include <veriblock/pop/value_sorted_map.hpp>

using namespace altintegration;

struct VSMTest : ::testing::Test {};

static auto cmp = [](const int& v1, const int& v2) -> bool { return v1 < v2; };

TEST_F(VSMTest, Basic_test) {
  ValueSortedMap<std::string, int> map{cmp};

  map.insert("hello", 5);
  map.insert("hello 2", 6);
  map.insert("hello", 10);

  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.empty());

  map.clear();

  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

TEST_F(VSMTest, find_test) {
  ValueSortedMap<int, int> map{cmp};

  map.insert(1, 5);
  map.insert(2, 6);
  map.insert(1, 10);

  EXPECT_EQ(map.find(3), map.end());

  auto it = map.find(2);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, 6);

  it = map.find(1);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, 10);
}

TEST_F(VSMTest, erase_test) {
  ValueSortedMap<std::string, int> map{cmp};

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

  map.erase("key 2");

  EXPECT_EQ(map.size(), 3);
  EXPECT_FALSE(map.empty());

  map.erase(map.find("key 1"));

  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.empty());
}

TEST_F(VSMTest, value_sort_test) {
  ValueSortedMap<std::string, int> map{cmp};

  map.insert("key 1", 4);
  map.insert("key 4", 2);
  map.insert("key 5", 6);
  map.insert("key 6", 1);
  map.insert("key 7", 12);
  map.insert("key 8", 4);
  map.insert("key 9", 6);

  auto s = map.getSortedValues();

  auto it = s.begin();
  EXPECT_EQ(*(it++), 1);
  EXPECT_EQ(*(it++), 2);
  EXPECT_EQ(*(it++), 4);
  EXPECT_EQ(*(it++), 4);
  EXPECT_EQ(*(it++), 6);
  EXPECT_EQ(*(it++), 6);
  EXPECT_EQ(*(it++), 12);

  ASSERT_TRUE(std::is_sorted(s.begin(), s.end()));
}