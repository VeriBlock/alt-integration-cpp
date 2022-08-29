// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/bfi/bitcoin/block.hpp"

#include <gtest/gtest.h>

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

  Serialize(writer, header);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f0864");

  BlockHeader decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(header, decoded);
}

TEST(Block, serde_test_witouth_pop_data) {
  TxIn in{
      OutPoint{uint256::fromHex("7b1eabe0209b1fe794124575ef807057c77ada2138"
                                "ae4fa8d6c4de0398a14f3f"),
               0},
      ParseHex("4830450221008949f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1"
               "a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268e082"
               "a725f8aeae0573fb12ff866a5f01"),
      0xffffffff,
      {ParseHex("ff4cff")},
  };
  TxOut out{(Amount)4999990000,
            ParseHex("76a914cbc20a7664f2f69e5355aa427045bc15e7c6c77288ac")};
  Transaction tx{{in}, {out}, 1, 0};

  Block block{
      2,
      uint256::fromHex(
          "b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000"),
      uint256::fromHex(
          "9d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab31471"),
      1415239972,
      404472624,
      1678286846};
  block.vtx = {tx};
  WriteStream writer;

  Serialize(writer, block);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f086401010000000001017b1eabe0209b1fe7941245"
            "75ef807057c77ada2138ae4fa8d6c4de0398a14f3f000000004948304502210089"
            "49f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1a7150deb388ab89350220"
            "79656090d7f6bac4c9a94e0aad311a4268e082a725f8aeae0573fb12ff866a5f01"
            "ffffffff01f0ca052a010000001976a914cbc20a7664f2f69e5355aa427045bc15"
            "e7c6c77288ac0103ff4cff00000000");

  Block decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(block, decoded);
}

TEST(BlockLocator, serde_test) {
  BlockLocator blockLocator{
      {uint256::fromHex(
           "d39f608a7775b537729884d4e6633bb2105e55a16a14d31b0000000000000000"),
       uint256::fromHex("5c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a000000"
                        "0000000000")}};
  WriteStream writer;
  writer.setVersion(70001);

  Serialize(writer, blockLocator);

  ASSERT_EQ(
      writer.hex(),
      "7111010002d39f608a7775b537729884d4e6633bb2105e55a16a14d31b00000000000000"
      "005c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a0000000000000000");

  BlockLocator decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(blockLocator, decoded);
}
