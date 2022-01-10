// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/bfi/bitcoin/block.hpp"

using namespace altintegration;
using namespace altintegration::btc;

TEST(BlockHeader, serde_test) {
  BlockHeader header{
      2,
      uint256::fromHex(
          "b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000"),
      uint256::fromHex(
          "9d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab31471"),
      1415239972,
      404472624,
      1678286846};
  WriteStream writer;

  SerializeBtc(writer, header);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f0864");

  BlockHeader decoded{};
  ReadStream reader{writer.data()};

  UnserializeBtc(reader, decoded);

  ASSERT_EQ(header, decoded);
}