// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/coin.hpp>

using namespace altintegration;

void Coin::toVbkEncoding(WriteStream& stream) const {
  writeSingleBEValue(stream, units);
}

size_t Coin::estimateSize() const { return singleBEValueSize(units); }

bool Coin::operator==(const Coin& other) const noexcept {
  return units == other.units;
}

std::string Coin::toPrettyString() const { return format("Coin{{{}}}", units); }

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                Coin& out,
                                                ValidationState& state) {
  int64_t amount = 0;
  if (!readSingleBEValue<int64_t>(stream, amount, state)) {
    return state.Invalid("invalid-amount");
  }
  out = Coin(amount);
  return true;
}
