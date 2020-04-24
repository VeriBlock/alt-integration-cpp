#include "veriblock/entities/altpoptx.hpp"

#include <gtest/gtest.h>

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

  AltPopTx expectedAltPopTx = {2, atv, {vtb}};
  std::vector<uint8_t> bytes = expectedAltPopTx.toVbkEncoding();

  AltPopTx encodedAltPopTx = AltPopTx::fromVbkEncoding(bytes);

  EXPECT_EQ(encodedAltPopTx, expectedAltPopTx);
}
