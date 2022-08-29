// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/pop_payouts.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const std::string defaultPopPayoutsEncoded =
    "010309616464726573735f31000000000000000a09616464726573735f3200000000000000"
    "d509616464726573735f33000000000000013a";

static const PopPayouts defaultPopPayouts{
    {{"address_1"_v, 10}, {"address_2"_v, 213}, {"address_3"_v, 314}}};

TEST(PopPayouts, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<PopPayouts>(defaultPopPayoutsEncoded);

  EXPECT_EQ(decoded.payouts, defaultPopPayouts.payouts);
}

TEST(PopPayouts, Serialize) {
  WriteStream stream;
  defaultPopPayouts.toVbkEncoding(stream);
  auto bytes = stream.data();
  auto encoded = HexStr(bytes);
  EXPECT_EQ(encoded, defaultPopPayoutsEncoded);
}

TEST(PopPayouts, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<PopPayouts>(defaultPopPayoutsEncoded);
  EXPECT_EQ(decoded.payouts, defaultPopPayouts.payouts);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  auto reEncoded = HexStr(bytes);
  EXPECT_EQ(reEncoded, defaultPopPayoutsEncoded);
}
