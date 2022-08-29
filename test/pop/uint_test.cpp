// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/uint.hpp>

using namespace altintegration;

TEST(uint256, uint256_compareTest) {
  uint256 hash1(
      "00000000000000000000000000000000000000000000000000000000000000ff"_unhex);
  uint256 hash2(
      "0000000000000000000000000000000000000000000000000000000000000fff"_unhex);

  ASSERT_TRUE(hash2 > hash1);
  ASSERT_TRUE(hash1 < hash2);

  hash1 = uint256(
      "00000000000000000000000000000000000000000000000000000000000033ff"_unhex);
  hash2 = uint256(
      "00000000000000000000000000000000000000000000000000000000000032ff"_unhex);

  ASSERT_TRUE(hash1 > hash2);
  ASSERT_TRUE(hash2 < hash1);

  hash1 = uint256(
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff322"_unhex);
  hash2 = uint256(
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff322"_unhex);

  ASSERT_TRUE(hash1 == hash2);
}

struct TestCase {
  uint32_t bits;
  uint256 target;
};

class BitsTest : public testing::TestWithParam<TestCase> {};

static std::vector<TestCase> decodeBits_cases = {
    {0x04800000,
     uint256::fromHex(
         "0000000000000000000000000000000000000000000000000000000000000000")},
    {0x01123456,
     uint256::fromHex(
         "1200000000000000000000000000000000000000000000000000000000000000")},
    {0x02123456,
     uint256::fromHex(
         "3412000000000000000000000000000000000000000000000000000000000000")},
    {0x03123456,
     uint256::fromHex(
         "5634120000000000000000000000000000000000000000000000000000000000")},
    {0x04123456,
     uint256::fromHex(
         "0056341200000000000000000000000000000000000000000000000000000000")},
    {0x05009234,
     uint256::fromHex(
         "0000349200000000000000000000000000000000000000000000000000000000")},
    {0x20123456,
     uint256::fromHex(
         "0000000000000000000000000000000000000000000000000000000000563412")}

};

TEST_P(BitsTest, uint256_decodeBits) {
  auto value = GetParam();
  uint256 target = ArithUint256::fromBits(value.bits);
  EXPECT_EQ(target, value.target);
}

INSTANTIATE_TEST_SUITE_P(BitsRegression,
                         BitsTest,
                         testing::ValuesIn(decodeBits_cases));
