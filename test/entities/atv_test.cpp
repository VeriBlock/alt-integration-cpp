#include "veriblock/entities/atv.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

static const std::string defaultAtvEncoded =
    "01580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100"
    "010c6865616465722062797465730112636f6e7465787420696e666f206279746573011170"
    "61796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e"
    "6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d"
    "8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a034200"
    "04de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692"
    "b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e040000000104000000"
    "00201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000"
    "00022000000000000000000000000000000000000000000000000000000000000000002000"
    "00000000000000000000000000000000000000000000000000000000000000400000138800"
    "02449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4"
    "462ef24ae02d67e47d785c9b90f301010000000000010100";

TEST(ATV, Deserialize) {
  const auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  auto decoded = ATV::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.transaction.sourceAddress, Address(AddressType::STANDARD, "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));
}

TEST(ATV, RoundTrip) {
  auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  auto decoded = ATV::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address(AddressType::STANDARD, "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultAtvEncoded);
}
