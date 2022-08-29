// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/bfi/bitcoin/net/messages.hpp"

#include <gtest/gtest.h>

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

TEST(FilterLoadMsg, serde_test) {
  FilterLoadMsg msg{{{0xb5, 0x0f}, 11, 0, 0}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(), "02b50f0b0000000000000000");

  FilterLoadMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(FilterAddMsg, serde_test) {
  FilterAddMsg msg{{0xfd, 0xac, 0xf9, 0xb3, 0xeb, 0x07, 0x74, 0x12,
                    0xe7, 0xa9, 0x68, 0xd2, 0xe4, 0xf1, 0x1b, 0x9a,
                    0x9d, 0xee, 0x31, 0x2d, 0x66, 0x61, 0x87, 0xed,
                    0x77, 0xee, 0x7d, 0x26, 0xaf, 0x16, 0xcb, 0x0b}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(
      writer.hex(),
      "20fdacf9b3eb077412e7a968d2e4f11b9a9dee312d666187ed77ee7d26af16cb0b");

  FilterAddMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(FeeFilterMsg, serde_test) {
  FeeFilterMsg msg{48508};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(), "7cbd000000000000");

  FeeFilterMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(NotFoundMsg, serde_test) {
  NotFoundMsg msg{
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

  NotFoundMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(AddrMsg, serde_test) {
  AddrMsg msg{{{{{0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0x00,
                  0xff,
                  0xff,
                  0xc0,
                  0x00,
                  0x02,
                  0x33},
                 8333},
                ServiceFlags::NODE_NETWORK,
                1414012889}}};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(),
            "01d91f4854010000000000000000000000000000000000ffffc0000233208d");

  AddrMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}

TEST(SendCmpctMsg, serde_test) {
  SendCmpctMsg msg{true, 0x121521};
  WriteStream writer;

  Serialize(writer, msg);

  ASSERT_EQ(writer.hex(), "012115120000000000");

  SendCmpctMsg decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(msg, decoded);
}
