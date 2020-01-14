#include "veriblock/entities/coin.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

TEST(Coin, RoundTrip) {
  Coin input{ 123456789L };
  WriteStream stream;
  Coin::toVbkEncoding(stream, input);
  auto coinEncoded = stream.data();
  ReadStream readStream{ coinEncoded };
  auto output = Coin::fromVbkEncoding(readStream);

  EXPECT_EQ(output.units, input.units);
}

TEST(Coin, Invalid) {
  std::vector<uint8_t> coinEncoded(9, 0xFF);
  ReadStream readStream{ coinEncoded };

  EXPECT_THROW(Coin::fromVbkEncoding(readStream), std::out_of_range);
}
