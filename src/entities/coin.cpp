#include "veriblock/entities/coin.hpp"

using namespace VeriBlock;

Coin Coin::fromVbkEncoding(ReadStream& stream) {
  return Coin(readSingleBEValue<int64_t>(stream));
}

void Coin::toVbkEncoding(WriteStream& stream) const {
  writeSingleBEValue(stream, units);
}

bool Coin::operator==(const Coin& other) const noexcept {
  return units == other.units;
}
