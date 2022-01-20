// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/bfi/bitcoin/net/messages.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/write_stream.hpp"

using namespace altintegration;
using namespace altintegration::btc;

TEST(InvMsg, serde_test) {
  InvMsg msg{{{1,
               uint256::fromHex("de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732"
                                "e429081da1b621f55a")},
              {1,
               uint256::fromHex("91d36d997037e08018262978766f24b8a055aaf1d872e9"
                                "4ae85e9817b2c68dc7")}}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(),
            "0201000000de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1"
            "b621f55a0100000091d36d997037e08018262978766f24b8a055aaf1d872e94ae8"
            "5e9817b2c68dc7");

  InvMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(GetDataMsg, serde_test) {
  GetDataMsg msg{
      {{1,
        uint256::fromHex("de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732"
                         "e429081da1b621f55a")},
       {1,
        uint256::fromHex("91d36d997037e08018262978766f24b8a055aaf1d872e9"
                         "4ae85e9817b2c68dc7")}}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(),
            "0201000000de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1"
            "b621f55a0100000091d36d997037e08018262978766f24b8a055aaf1d872e94ae8"
            "5e9817b2c68dc7");

  GetDataMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(GetBlocksMsg, serde_test) {
  GetBlocksMsg msg{
      {{uint256::fromHex(
            "d39f608a7775b537729884d4e6633bb2105e55a16a14d31b0000000000000000"),
        uint256::fromHex(
            "5c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a000000"
            "0000000000")}},
      uint256::fromHex(
          "0000000000000000000000000000000000000000000000000000000000000000")};
  WriteStream writer;
  writer.setVersion(70001);

  Serialize(writer, msg);

  ASSERT_EQ(
      writer.hex(),
      "7111010002d39f608a7775b537729884d4e6633bb2105e55a16a14d31b00000000000000"
      "005c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a0000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000");

  GetBlocksMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(GetBlockTxnMsg, serde_test) {
  GetBlockTxnMsg msg{
      {uint256::fromHex(
           "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a"),
       {1, 2, 3, 4, 5}}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(),
            "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a05"
            "0100000000");

  GetBlockTxnMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(GetHeadersMsg, serde_test) {
  GetHeadersMsg msg{
      {{uint256::fromHex(
            "d39f608a7775b537729884d4e6633bb2105e55a16a14d31b0000000000000000"),
        uint256::fromHex(
            "5c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a000000"
            "0000000000")}},
      uint256::fromHex(
          "0000000000000000000000000000000000000000000000000000000000000000")};
  WriteStream writer;
  writer.setVersion(70001);

  Serialize(writer, msg);

  ASSERT_EQ(
      writer.hex(),
      "7111010002d39f608a7775b537729884d4e6633bb2105e55a16a14d31b00000000000000"
      "005c3e6403d40837110a2e8afb602b1c01714bda7ce23bea0a0000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000");

  GetHeadersMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(TxMsg, serde_test) {
  TxMsg msg{{{}, {}, 1, 0}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(), "01000000000000000000");

  TxMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(CmpctBlockMsg, serde_test) {
  CmpctBlockMsg msg{};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(),
            "000000000000000000000000000000000000000000000000000000000000000000"
            "000000000000000000000000000000000000000000000000000000000000000000"
            "000000000000000000000000000000000000000000000000");

  CmpctBlockMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(BlockTxnMsg, serde_test) {
  BlockTxnMsg msg{
      {uint256::fromHex(
           "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a"),
       {}}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(
      writer.hex(),
      "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a00");

  BlockTxnMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}