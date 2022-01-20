// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/coin.hpp>

#include <gtest/gtest.h>

#include <veriblock/pop/literals.hpp>

using namespace altintegration;

TEST(Coin, Serialize) {
  Coin input(3500000000);
  WriteStream stream;
  input.toVbkEncoding(stream);
  auto coinEncoded = stream.data();
  EXPECT_EQ(HexStr(coinEncoded), "04d09dc300");
}

TEST(Coin, DeserializeFromVbkEncoding) {
  auto output = AssertDeserializeFromHex<Coin>("04d09dc300");
  EXPECT_EQ(output.units, 3500000000);
}

TEST(Coin, RoundTrip) {
  Coin input(123456789);
  WriteStream stream;
  input.toVbkEncoding(stream);
  auto coinEncoded = stream.data();
  auto output = AssertDeserializeFromVbkEncoding<Coin>(coinEncoded);

  EXPECT_EQ(output.units, input.units);
}

TEST(Coin, Invalid) {
  std::vector<uint8_t> coinEncoded(9, 0xFF);
  ReadStream readStream(coinEncoded);
  ValidationState state;
  Coin coin;
  ASSERT_FALSE(DeserializeFromVbkEncoding(readStream, coin, state));
  ASSERT_EQ(
      state.GetPath(),
      "invalid-amount+readsinglebe-bad-data+readsingle-bad-range+range-above");
}
