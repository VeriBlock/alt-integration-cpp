// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/bfi/bitcoin/transaction.hpp"

using namespace altintegration;
using namespace altintegration::btc;

TEST(Transaction, serde_test) {
  Transaction tx{};
  WriteStream writer;

  SerializeBtc(writer, tx);

  Transaction decoded{};
  ReadStream reader{writer.data()};

  UnserializeBtc(reader, decoded);

  ASSERT_EQ(tx, decoded);
}