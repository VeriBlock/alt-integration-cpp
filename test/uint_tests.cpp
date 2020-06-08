// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "veriblock/literals.hpp"
#include "veriblock/uint.hpp"

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
     uint256(
         "0000000000000000000000000000000000000000000000000000000000000000"_unhex)},
    {0x01123456,
     uint256(
         "0000000000000000000000000000000000000000000000000000000000000012"_unhex)},
    {0x02123456,
     uint256(
         "0000000000000000000000000000000000000000000000000000000000001234"_unhex)},
    {0x03123456,
     uint256(
         "0000000000000000000000000000000000000000000000000000000000123456"_unhex)},
    {0x04123456,
     uint256(
         "0000000000000000000000000000000000000000000000000000000012345600"_unhex)},
    {0x05009234,
     uint256(
         "0000000000000000000000000000000000000000000000000000000092340000"_unhex)},
    {0x20123456,
     uint256(
         "1234560000000000000000000000000000000000000000000000000000000000"_unhex)}

};

TEST_P(BitsTest, uint256_decodeBits) {
  auto value = GetParam();
  uint256 target = decodeBits(value.bits);
  EXPECT_EQ(target, value.target);
}

// TODO: add "encode bits"

INSTANTIATE_TEST_SUITE_P(BitsRegression,
                         BitsTest,
                         testing::ValuesIn(decodeBits_cases));
