// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/logger.hpp>

TEST(Fmt, basic_test) {
  EXPECT_EQ(altintegration::format("{}, {}, {}", "hello", "world", 5),
            "hello, world, 5");
}

TEST(Fmt, invalid_format) {
  EXPECT_NO_THROW(altintegration::format("digit value: {}"));
}