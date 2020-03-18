#include "veriblock/entities/output.hpp"

using namespace AltIntegrationLib;

bool Output::operator==(const Output& other) const noexcept {
  return address == other.address && coin == other.coin;
}

Output Output::fromVbkEncoding(ReadStream& stream) {
  auto addr = Address::fromVbkEncoding(stream);
  auto amount = Coin::fromVbkEncoding(stream);
  return Output(addr, amount);
}

void Output::toVbkEncoding(WriteStream& stream) const {
  address.toVbkEncoding(stream);
  coin.toVbkEncoding(stream);
}
