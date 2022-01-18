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