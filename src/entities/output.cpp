// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/output.hpp"

using namespace altintegration;

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

std::string Output::toPrettyString() const {
  return fmt::sprintf(
      "Output{address=%s, coin=%lld}", address.toString(), coin.units);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 Output& out,
                                 ValidationState& state) {
  Address address;
  Coin amount;
  if (!Deserialize(stream, address, state)) {
    return state.Invalid("invalid-address");
  }
  if (!Deserialize(stream, amount, state)) {
    return state.Invalid("invalid-amount");
  }
  out = Output(address, amount);
  return true;
}
