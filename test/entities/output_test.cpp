#include <gtest/gtest.h>

#include <string>

#include "util/literals.hpp"
#include "veriblock/entities/output.hpp"

using namespace AltIntegrationLib;

static const auto OUTPUT_BYTES =
    "01166772F51AB208D32771AB1506970EEB664462730B838E020539"_unhex;

static const Output OUTPUT_VALUE =
    Output(Address::fromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"), Coin(1337));

TEST(Output, Deserialize) {
  auto stream = ReadStream(OUTPUT_BYTES);
  auto output = Output::fromVbkEncoding(stream);

  EXPECT_EQ(output, OUTPUT_VALUE);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(Output, Serialize) {
  auto stream = WriteStream();
  OUTPUT_VALUE.toVbkEncoding(stream);
  EXPECT_EQ(stream.data(), OUTPUT_BYTES);
}

TEST(Output, RoundTrip) {
  auto stream = ReadStream(OUTPUT_BYTES);
  auto decoded = Output::fromVbkEncoding(stream);
  EXPECT_EQ(decoded, OUTPUT_VALUE);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  EXPECT_EQ(bytes, OUTPUT_BYTES);
}
