// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
