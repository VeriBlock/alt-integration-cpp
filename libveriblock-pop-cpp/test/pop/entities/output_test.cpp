// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/output.hpp>

#include <gtest/gtest.h>

#include <string>

#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const auto OUTPUT_BYTES =
    "01166772F51AB208D32771AB1506970EEB664462730B838E020539"_unhex;

static const Output OUTPUT_VALUE = Output(
    Address::assertFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"), Coin(1337));

TEST(Output, DeserializeFromVbkEncoding) {
  auto output = AssertDeserializeFromVbkEncoding<Output>(OUTPUT_BYTES);
  EXPECT_EQ(output, OUTPUT_VALUE);
}

TEST(Output, Serialize) {
  auto stream = WriteStream();
  OUTPUT_VALUE.toVbkEncoding(stream);
  EXPECT_EQ(stream.data(), OUTPUT_BYTES);
}

TEST(Output, RoundTrip) {
  auto decoded = AssertDeserializeFromVbkEncoding<Output>(OUTPUT_BYTES);
  EXPECT_EQ(decoded, OUTPUT_VALUE);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  EXPECT_EQ(bytes, OUTPUT_BYTES);
}
