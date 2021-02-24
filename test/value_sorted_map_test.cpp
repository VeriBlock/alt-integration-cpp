// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <string>

#include "veriblock/value_sorted_map.hpp"

using namespace altintegration;

struct IntCmp {
  bool operator()(int a, int b) { return a < b; }
};

TEST(ValueSortedMap, Basic_test) {
  ValueSortedMap<std::string, int, IntCmp> map;

  map.insert("hello", 5);
  map.insert("hello 2", 6);
}