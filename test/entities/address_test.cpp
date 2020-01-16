#include "veriblock/entities/address.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

// standard address
static auto ADDRESS_BYTES =
    "01166772F51AB208D32771AB1506970EEB664462730B838E"_unhex;
static std::string ADDRESS_VALUE = "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX";

TEST(Address, Deserialize) {
  // TODO: add test for multisig addr
  auto stream = ReadStream(ADDRESS_BYTES);
  auto address = Address::fromVbkEncoding(stream);

  EXPECT_EQ(address, ADDRESS_VALUE);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(Address, Serialize) {
  // TODO: add test for multisig addr
  auto address = Address(AddressType::STANDARD, ADDRESS_VALUE);
  auto stream = WriteStream();

  address.toVbkEncoding(stream);

  EXPECT_EQ(stream.data(), ADDRESS_BYTES);
}

TEST(Address, RoundTrip) {
  auto stream = ReadStream(ADDRESS_BYTES);
  auto decoded = Address::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.data(), ADDRESS_VALUE);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  EXPECT_EQ(bytes, ADDRESS_BYTES);
}
