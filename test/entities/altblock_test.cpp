// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/altblock.hpp"

#include <gtest/gtest.h>

#include "veriblock/literals.hpp"

using namespace altintegration;

static const AltBlock defaultBlock{
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    {},
    156,
    1466};

TEST(AltBlock, RoundTrip) {
  std::vector<uint8_t> bytes = defaultBlock.toVbkEncoding();
  AltBlock deserializedBlock =
      AltBlock::fromVbkEncoding(std::string(bytes.begin(), bytes.end()));

  EXPECT_EQ(deserializedBlock.getHash(), defaultBlock.getHash());
  EXPECT_EQ(deserializedBlock.getHeight(), defaultBlock.getHeight());
  EXPECT_EQ(deserializedBlock.getBlockTime(), defaultBlock.getBlockTime());
}
