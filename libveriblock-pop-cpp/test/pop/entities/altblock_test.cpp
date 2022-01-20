// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/blockchain/alt_chain_params.hpp>

#include <gtest/gtest.h>

#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const AltBlock defaultBlock{
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    "1aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    156,
    1466};

TEST(AltBlock, RoundTrip) {
  std::vector<uint8_t> bytes = defaultBlock.toRaw();
  AltBlock deserializedBlock = AssertDeserializeFromRaw<AltBlock>(bytes);

  EXPECT_EQ(deserializedBlock.getHash(), defaultBlock.getHash());
  EXPECT_EQ(deserializedBlock.height, defaultBlock.height);
  EXPECT_EQ(deserializedBlock.getBlockTime(), defaultBlock.getBlockTime());
  EXPECT_EQ(deserializedBlock.getPreviousBlock(),
            defaultBlock.getPreviousBlock());
}
