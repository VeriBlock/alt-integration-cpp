// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/output.hpp>


#include "veriblock/pop/entities/address.hpp"
#include "veriblock/pop/entities/coin.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {
class WriteStream;
struct ReadStream;
}  // namespace altintegration

using namespace altintegration;

bool Output::operator==(const Output& other) const noexcept {
  return address == other.address && coin == other.coin;
}

void Output::toVbkEncoding(WriteStream& stream) const {
  address.toVbkEncoding(stream);
  coin.toVbkEncoding(stream);
}

size_t Output::estimateSize() const {
  size_t size = 0;
  size += address.estimateSize();
  size += coin.estimateSize();
  return size;
}

std::string Output::toPrettyString() const {
  return format(
      "Output{{address={}, coin={}}}", address.toString(), coin.units);
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
