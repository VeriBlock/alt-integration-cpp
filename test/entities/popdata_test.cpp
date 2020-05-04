#include <gtest/gtest.h>

#include <veriblock/entities/popdata.hpp>

#include "util/literals.hpp"
#include "util/test_utils.hpp"

using namespace altintegration;

TEST(AltPopTx, Deserialize) {
  const auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  ATV atv = ATV::fromVbkEncoding(stream);

  const auto vtbBytes = ParseHex(defaultVtbEncoded);
  stream = ReadStream(vtbBytes);
  VTB vtb = VTB::fromVbkEncoding(stream);

  PopData expectedPopData = {2, {}, true, atv, {vtb}};
  std::vector<uint8_t> bytes = expectedPopData.toVbkEncoding();

  PopData encodedPopData = PopData::fromVbkEncoding(bytes);

  EXPECT_EQ(encodedPopData, expectedPopData);
}
