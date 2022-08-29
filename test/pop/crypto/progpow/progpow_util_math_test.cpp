// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#undef VBK_HAS_BUILTIN_POPCOUNT
#undef VBK_HAS_BUILTIN_CLZ
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/crypto/progpow/math.hpp>

using namespace altintegration::progpow;

TEST(ProgPowMath, MulHi) {
  ASSERT_EQ(0UL, mul_hi(5, 100));
  ASSERT_EQ(0UL, mul_hi(0xffff, 0xffff));
  ASSERT_EQ(1UL, mul_hi(0xffff + 1, 0xffff + 1));
  // hi         lo
  // 0xfffffffe 00000001
  ASSERT_EQ(0xfffffffeUL, mul_hi(0xffffffff, 0xffffffff));
}

TEST(ProgPowMath, Clz) {
  ASSERT_EQ(32UL, clz(0));
  ASSERT_EQ(31UL, clz(1));
  ASSERT_EQ(29UL, clz(4));
  ASSERT_EQ(27UL, clz(16));
  ASSERT_EQ(24UL, clz(128));
  ASSERT_EQ(21UL, clz(1024));
  ASSERT_EQ(16UL, clz(0x0000ffff));
  ASSERT_EQ(8UL, clz(0x00ffffff));
  ASSERT_EQ(0UL, clz(0xffffffff));
}

TEST(ProgPowMath, Popcount) {
  ASSERT_EQ(0UL, popcount(0));
  ASSERT_EQ(1UL, popcount(1));
  ASSERT_EQ(8UL, popcount(0xff));
  ASSERT_EQ(24UL, popcount(0xffffff));
  ASSERT_EQ(32UL, popcount(0xffffffff));
}

TEST(ProgPowMath, ProgPowMath) {
  using namespace altintegration::progpow;
  ASSERT_EQ(44755968UL, math(0xaabb, 0xffaa, 0));
  ASSERT_EQ(43690UL, math(0xaabb, 0xffaa, 1));
  ASSERT_EQ(109157UL, math(0xaabb, 0xffaa, 2));
  ASSERT_EQ(22UL, math(0xaabb, 0xffaa, 3));
  ASSERT_EQ(32UL, math(0xaabb, 0xffaa, 4));
  ASSERT_EQ(2931818538UL, math(0xaabb, 0xffaa, 5));
  ASSERT_EQ(0UL, math(0xaabb, 0xffaa, 6));
  ASSERT_EQ(65467UL, math(0xaabb, 0xffaa, 7));
  ASSERT_EQ(2860623150UL, math(0xaabb, 0xffaa, 8));
  ASSERT_EQ(21777UL, math(0xaabb, 0xffaa, 9));
  ASSERT_EQ(43707UL, math(0xaabb, 0xffaa, 10));
}
