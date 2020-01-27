#include "veriblock/address.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

// standard address
static auto ADDRESS_BYTES =
    "01166772F51AB208D32771AB1506970EEB664462730B838E"_unhex;
static std::string ADDRESS_VALUE = "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX";

TEST(Address, Deserialize) {
  auto stream = ReadStream(ADDRESS_BYTES);
  auto address = addressFromVbkEncoding(stream);

  EXPECT_EQ(address, ADDRESS_VALUE);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(Address, Serialize) {
  auto address = AddressEntity(AddressType::STANDARD, ADDRESS_VALUE);
  auto stream = WriteStream();

  addressToVbkEncoding(address, stream);

  EXPECT_EQ(stream.data(), ADDRESS_BYTES);
}

TEST(Address, RoundTrip) {
  auto stream = ReadStream(ADDRESS_BYTES);
  auto decoded = addressFromVbkEncoding(stream);
  EXPECT_EQ(decoded.getAddr(), ADDRESS_VALUE);

  WriteStream outputStream;
  addressToVbkEncoding(decoded, outputStream);
  auto bytes = outputStream.data();
  EXPECT_EQ(bytes, ADDRESS_BYTES);
}

TEST(Address, ValidStandard) {
  std::string addressString = "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d";
  AddressEntity address = addressFromString(addressString);
  EXPECT_EQ(address.getType(), AddressType::STANDARD);
}
