// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <ostream>
#include <string>
#include <vector>
#include <veriblock/pop/strutil.hpp>

struct TestCase {
  std::vector<uint8_t> binData{};
  std::string baseData{};
};

class Base58Test : public testing::TestWithParam<TestCase> {};

static std::vector<TestCase> g_Cases = {
    {
        {},
        "",
    },

    {
        {27, 53, 84, 6, 78, 51, 29, 38, 57, 2,  0,  95, 23,
         89, 4,  6,  7, 92, 35, 61, 30, 95, 12, 96, 29},
        "BwzG4EVMJLbJdA9WPgQW2hxeFB9v31nYFE",
    },

    {
        {2, 9, 5, 7, 9, 3, 4, 5, 7, 3, 8, 9, 4, 6, 7, 3, 7,
         8, 9, 6, 7, 9, 5, 7, 2, 9, 8, 5, 2, 9, 8, 5, 7, 2},
        "3fmxeMiA19wm9A3ReBdfMQRQSQ6nkXWnBWHRM83DZMukjK",
    },

    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        "1111111111111111111111111111111111",
    },
};

TEST_P(Base58Test, Encode) {
  auto tc = GetParam();
  EXPECT_EQ(altintegration::EncodeBase58(tc.binData), tc.baseData);
}

TEST_P(Base58Test, Decode) {
  auto tc = GetParam();
  std::vector<uint8_t> decoded;
  altintegration::ValidationState state;
  EXPECT_TRUE(altintegration::DecodeBase58(tc.baseData, decoded, state))
      << state.toString();
  EXPECT_EQ(decoded, tc.binData);
}

INSTANTIATE_TEST_SUITE_P(Base58Regression,
                         Base58Test,
                         testing::ValuesIn(g_Cases));
