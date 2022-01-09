// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/abfi/bitcoin/block.hpp"

using namespace btc;
using namespace altintegration;

TEST(BlockHeader, serde_test) {
  BlockHeader header{2, uint256{}, uint256{}, 618224212, 818092824, 4271835236};
  WriteStream writer;

  SerializeBtc(writer, header);

  BlockHeader decoded{};
  ReadStream reader{writer.data()};

  UnserializeBtc(reader, decoded);

  ASSERT_EQ(header, decoded);
}