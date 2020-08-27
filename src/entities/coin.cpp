// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/coin.hpp"

using namespace altintegration;

Coin Coin::fromVbkEncoding(ReadStream& stream) {
  return Coin(readSingleBEValue<int64_t>(stream));
}

void Coin::toVbkEncoding(WriteStream& stream) const {
  writeSingleBEValue(stream, units);
}

bool Coin::operator==(const Coin& other) const noexcept {
  return units == other.units;
}

std::string Coin::toPrettyString() const {
  return fmt::sprintf("Coin{%lld}", units);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 Coin& out,
                                 ValidationState& state) {
  int64_t amount;
  if (!readSingleBEValueNoExcept<int64_t>(stream, amount, state)) {
    return state.Invalid("invalid-amount");
  }
  out = Coin(amount);
  return true;
}

bool altintegration::Deserialize(Slice<const uint8_t> data,
                                 Coin& out,
                                 ValidationState& state) {
  ReadStream stream(data);
  return Deserialize(stream, out, state);
}
