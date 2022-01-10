// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <veriblock/pop/crypto/progpow/ethash.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

using namespace altintegration;

class EthashDagSeedTest : public testing::TestWithParam<int> {};

using namespace altintegration;

TEST_P(EthashDagSeedTest, Regression) {
  auto blockNumber = GetParam();
  auto expected = progpow::ethash_calculate_seedhash(blockNumber);
  auto actual = progpow::ethash_get_seedhash(blockNumber);
  ASSERT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(EthashDagSeedRegression,
                         EthashDagSeedTest,
                         testing::Range(0, 4095 * 8000, 8000));
