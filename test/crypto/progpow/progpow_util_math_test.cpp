// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#undef VBK_HAS_BUILTIN_POPCOUNT
#undef VBK_HAS_BUILTIN_CLZ
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/crypto/progpow/math.hpp>

using namespace altintegration::progpow;

TEST(ProgPowMath, MulHi) {
  ASSERT_EQ(0, mul_hi(5, 100));
  ASSERT_EQ(0, mul_hi(0xffff, 0xffff));
  ASSERT_EQ(1, mul_hi(0xffff + 1, 0xffff + 1));
  // hi         lo
  // 0xfffffffe 00000001
  ASSERT_EQ(0xfffffffe, mul_hi(0xffffffff, 0xffffffff));
}

TEST(ProgPowMath, Clz) {
  ASSERT_EQ(32, clz(0));
  ASSERT_EQ(31, clz(1));
  ASSERT_EQ(29, clz(4));
  ASSERT_EQ(27, clz(16));
  ASSERT_EQ(24, clz(128));
  ASSERT_EQ(21, clz(1024));
  ASSERT_EQ(16, clz(0x0000ffff));
  ASSERT_EQ(8, clz(0x00ffffff));
  ASSERT_EQ(0, clz(0xffffffff));
}

TEST(ProgPowMath, Popcount) {
  ASSERT_EQ(0, popcount(0));
  ASSERT_EQ(1, popcount(1));
  ASSERT_EQ(8, popcount(0xff));
  ASSERT_EQ(24, popcount(0xffffff));
  ASSERT_EQ(32, popcount(0xffffffff));
}

TEST(ProgPowMath, ProgPowMath) {
  using namespace altintegration::progpow;
  ASSERT_EQ(44755968, math(0xaabb, 0xffaa, 0));
  ASSERT_EQ(43690, math(0xaabb, 0xffaa, 1));
  ASSERT_EQ(109157, math(0xaabb, 0xffaa, 2));
  ASSERT_EQ(22, math(0xaabb, 0xffaa, 3));
  ASSERT_EQ(32, math(0xaabb, 0xffaa, 4));
  ASSERT_EQ(2931818538, math(0xaabb, 0xffaa, 5));
  ASSERT_EQ(0, math(0xaabb, 0xffaa, 6));
  ASSERT_EQ(65467, math(0xaabb, 0xffaa, 7));
  ASSERT_EQ(2860623150, math(0xaabb, 0xffaa, 8));
  ASSERT_EQ(21777, math(0xaabb, 0xffaa, 9));
  ASSERT_EQ(43707, math(0xaabb, 0xffaa, 10));
}