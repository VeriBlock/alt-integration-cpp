// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/bfi/bitcoin/net/net_entities.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/write_stream.hpp"

using namespace altintegration;
using namespace altintegration::btc;

TEST(NetAddr, serde_test) {
  NetAddr net_addr{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
  WriteStream writer;

  Serialize(writer, net_addr);

  ASSERT_EQ(writer.hex(), "0102030405060708090a0b0c0d0e0f10");

  NetAddr decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(net_addr, decoded);
}

TEST(SubNet, serde_test) {
  NetAddr net_addr{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
  SubNet sub_net{
      net_addr, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, true};
  WriteStream writer;

  Serialize(writer, sub_net);

  ASSERT_EQ(
      writer.hex(),
      "0102030405060708090a0b0c0d0e0f100102030405060708090a0b0c0d0e0f1001");

  SubNet decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(sub_net, decoded);
}

TEST(Service, serde_test) {
  Service service{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, 255};
  WriteStream writer;

  Serialize(writer, service);

  ASSERT_EQ(writer.hex(), "0102030405060708090a0b0c0d0e0f1000ff");

  Service decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(service, decoded);
}

TEST(BanEntry, serde_test) {
  BanEntry ban_entry{255, 01, 02, 255};
  WriteStream writer;

  Serialize(writer, ban_entry);

  ASSERT_EQ(writer.hex(), "ff00000001000000000000000200000000000000ff");

  BanEntry decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(ban_entry, decoded);
}

TEST(Inv, serde_test) {
  Inv inv{
      1,
      uint256::fromHex(
          "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a")};
  WriteStream writer;

  Serialize(writer, inv);

  ASSERT_EQ(writer.hex(),
            "01000000de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b6"
            "21f55a");

  Inv decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(inv, decoded);
}

TEST(BlockTransactionsRequest, serde_test) {
  BlockTransactionsRequest req{
      uint256::fromHex(
          "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a"),
      {1, 2, 3, 4, 5}};
  WriteStream writer;

  Serialize(writer, req);

  ASSERT_EQ(writer.hex(),
            "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a05"
            "0100000000");

  BlockTransactionsRequest decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(req, decoded);
}

TEST(PrefilledTransaction, serde_test) {
  PrefilledTransaction tx{15, {}};
  WriteStream writer;

  Serialize(writer, tx);

  ASSERT_EQ(writer.hex(), "0f00000000000000000000");

  PrefilledTransaction decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(tx, decoded);
}

TEST(BlockHeaderAndShortTxIDs, serde_test) {
  BlockHeaderAndShortTxIDs block_header{{}, 15, {}, {}};
  WriteStream writer;

  Serialize(writer, block_header);

  ASSERT_EQ(writer.hex(),
            "000000000000000000000000000000000000000000000000000000000000000000"
            "000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000f000000000000000000");

  BlockHeaderAndShortTxIDs decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(block_header, decoded);
}

TEST(BlockTransactions, serde_test) {
  BlockTransactions txs{
      uint256::fromHex(
          "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a"),
      {}};
  WriteStream writer;

  Serialize(writer, txs);

  ASSERT_EQ(
      writer.hex(),
      "de55ffd709ac1f5dc509a0925d0b1fc442ca034f224732e429081da1b621f55a00");

  BlockTransactions decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(txs, decoded);
}