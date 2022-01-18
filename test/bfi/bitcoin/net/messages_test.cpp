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