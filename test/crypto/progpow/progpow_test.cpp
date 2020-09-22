// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/crypto/progpow.hpp>
#include <veriblock/literals.hpp>
#include <veriblock/uint.hpp>

using namespace altintegration;

struct TestCase {
  std::string header;
  std::string hash;
};

// clang-format off
static std::vector<TestCase> cases = {
    // header (65 bytes), progpow hash (48 bytes)
    {"000F42400002449C60619294546AD825AF03B0935637860679DDD55EE4FD21082E18686E26BBFDA7D5E4462EF24AE02D67E47D785C9B90F3010100000000000005", "62FC2885599FB95310B935EC949666A5EFF6364D6D6A28AD"}
};
// clang-format on

class ProgPowBlockHashTest : public testing::TestWithParam<TestCase> {};

TEST_P(ProgPowBlockHashTest, Regression) {
  auto [strheader, strhash] = GetParam();

  std::vector<uint8_t> header = ParseHex(strheader);
  uint192 expected = uint192::fromHexStrict(strhash);

  auto actual = progPowHash(header);
  ASSERT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(ProgPowBlockHashRegression,
                         ProgPowBlockHashTest,
                         testing::ValuesIn(cases));
