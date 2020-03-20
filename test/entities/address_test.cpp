#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "util/literals.hpp"
#include "veriblock/entities/address.hpp"

using namespace altintegration;

// standard address
static auto ADDRESS_BYTES =
    "01166772F51AB208D32771AB1506970EEB664462730B838E"_unhex;
static std::string ADDRESS_VALUE = "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX";

TEST(Address, Deserialize) {
  auto stream = ReadStream(ADDRESS_BYTES);
  auto address = Address::fromVbkEncoding(stream);

  EXPECT_EQ(address.toString(), ADDRESS_VALUE);
  EXPECT_EQ(address.getType(), AddressType::STANDARD);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(Address, Serialize) {
  auto address = Address::fromString(ADDRESS_VALUE);
  auto stream = WriteStream();

  address.toVbkEncoding(stream);

  EXPECT_EQ(stream.data(), ADDRESS_BYTES);
}

TEST(Address, RoundTrip) {
  auto stream = ReadStream(ADDRESS_BYTES);
  auto decoded = Address::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.toString(), ADDRESS_VALUE);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  EXPECT_EQ(bytes, ADDRESS_BYTES);
}

TEST(Address, ValidStandard) {
  std::string addressString = "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d";
  Address address = Address::fromString(addressString);
  EXPECT_EQ(address.getType(), AddressType::STANDARD);
  EXPECT_EQ(address.toString(), addressString);
}

TEST(Address, ValidMultisig) {
  std::string addressString = "V23Cuyc34u5rdk9psJ86aFcwhB1md0";
  Address address = Address::fromString(addressString);
  EXPECT_EQ(address.getType(), AddressType::MULTISIG);
  EXPECT_EQ(address.toString(), addressString);
}

TEST(Address, DerivedFromPublicKey) {
  auto publicKey =
      "3056301006072a8648ce3d020106052b8104000a03420004cb427e41a0114874080"
      "a4b1e2ab7920e22cd2d188c87140defa447ee5fc44bb848e1c0db5ef206de2e7002"
      "f6c86952be4823a4c08e65e4cdbeb904a8b95763aa"_unhex;
  std::string addressString = "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d";
  Address address = Address::fromString(addressString);
  EXPECT_TRUE(address.isDerivedFromPublicKey(publicKey));
}

TEST(Address, NotDerivedFromPublicKey) {
  auto publicKey =
      "3056301006072a8648ce3d020106052b8104000a03420004cb427e41a0114874080"
      "a4b1e2ab7920e22cd2d188c87140defa447ee5fc44bb848e1c0db5ef206de2e7002"
      "f6c86952be4823a4c08e65e4cdbeb904a8b95763aa"_unhex;
  std::string addressString = "V23Cuyc34u5rdk9psJ86aFcwhB1md0";
  Address address = Address::fromString(addressString);
  EXPECT_FALSE(address.isDerivedFromPublicKey(publicKey));
}

TEST(Address, ParseStandard) {
  std::string addressString = "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d";
  Address address = Address::fromString(addressString);
  WriteStream outputStream;
  address.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  auto stream = ReadStream(bytes);
  auto decoded = Address::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.toString(), addressString);
  EXPECT_EQ(decoded.getType(), AddressType::STANDARD);
  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(Address, ParseMultisig) {
  std::string addressString = "V23Cuyc34u5rdk9psJ86aFcwhB1md0";
  Address address = Address::fromString(addressString);
  WriteStream outputStream;
  address.toVbkEncoding(outputStream);
  auto bytes = outputStream.data();
  auto stream = ReadStream(bytes);
  auto decoded = Address::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.toString(), addressString);
  EXPECT_EQ(decoded.getType(), AddressType::MULTISIG);
  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}
