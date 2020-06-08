// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/coin.hpp"

#include <gtest/gtest.h>

#include "veriblock/literals.hpp"

using namespace altintegration;

TEST(Coin, Serialize) {
  Coin input(3500000000);
  WriteStream stream;
  input.toVbkEncoding(stream);
  auto coinEncoded = stream.data();
  EXPECT_EQ(HexStr(coinEncoded), "04d09dc300");
}

TEST(Coin, Deserialize) {
  auto coinBytes = "04d09dc300"_unhex;
  ReadStream readStream(coinBytes);
  auto output = Coin::fromVbkEncoding(readStream);

  EXPECT_EQ(output.units, 3500000000);
}

TEST(Coin, RoundTrip) {
  Coin input(123456789);
  WriteStream stream;
  input.toVbkEncoding(stream);
  auto coinEncoded = stream.data();
  ReadStream readStream(coinEncoded);
  auto output = Coin::fromVbkEncoding(readStream);

  EXPECT_EQ(output.units, input.units);
}

TEST(Coin, Invalid) {
  std::vector<uint8_t> coinEncoded(9, 0xFF);
  ReadStream readStream(coinEncoded);

  EXPECT_THROW(Coin::fromVbkEncoding(readStream), std::out_of_range);
}
