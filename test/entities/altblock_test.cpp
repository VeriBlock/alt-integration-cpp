#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/altblock.hpp"

using namespace altintegration;

static const AltBlock defaultBlock{
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    {},
    156,
    1466};

TEST(AltBlock, RoundTrip) {
  std::vector<uint8_t> bytes = defaultBlock.toVbkEncoding();
  AltBlock deserializedBlock =
      AltBlock::fromVbkEncoding(std::string(bytes.begin(), bytes.end()));

  EXPECT_EQ(deserializedBlock.hash, defaultBlock.hash);
  EXPECT_EQ(deserializedBlock.height, defaultBlock.height);
  EXPECT_EQ(deserializedBlock.timestamp, defaultBlock.timestamp);
}
