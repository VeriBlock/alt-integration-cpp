// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/output.hpp"

using namespace altintegration;

bool Output::operator==(const Output& other) const noexcept {
  return address == other.address && coin == other.coin;
}

void Output::toVbkEncoding(WriteStream& stream) const {
  address.toVbkEncoding(stream);
  coin.toVbkEncoding(stream);
}

std::string Output::toPrettyString() const {
  return fmt::sprintf(
      "Output{address=%s, coin=%lld}", address.toString(), coin.units);
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                 Output& out,
                                 ValidationState& state) {
  Address address;
  Coin amount;
  if (!DeserializeFromVbkEncoding(stream, address, state)) {
    return state.Invalid("output-address");
  }
  if (!DeserializeFromVbkEncoding(stream, amount, state)) {
    return state.Invalid("output-amount");
  }
  out = Output(address, amount);
  return true;
}
