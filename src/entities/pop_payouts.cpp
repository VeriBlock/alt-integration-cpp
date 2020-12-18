// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/pop_payouts.hpp"

namespace altintegration {

void PopPayoutValue::toVbkEncoding(WriteStream& stream) const {
  writeSingleByteLenValue(stream, address);
  stream.writeBE<int64_t>(amount);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopPayoutValue& out,
                                ValidationState& state) {
  if (!readSingleByteLenValue<std::vector<uint8_t>>(
          stream, out.address, state, 0)) {
    return state.Invalid("pop-payout-value-address");
  }

  if (!stream.readBE<int64_t>(out.amount, state)) {
    return state.Invalid("pop-payout-value-amount");
  }
  return true;
}

bool operator==(const PopPayoutValue& a, const PopPayoutValue& b) {
  return a.address == b.address && a.amount == b.amount;
}

bool operator!=(const PopPayoutValue& a, const PopPayoutValue& b) {
  return !(a == b);
}

void PopPayouts::toVbkEncoding(WriteStream& stream) const {
  writeArrayOf<PopPayoutValue>(
      stream, values, [](WriteStream& stream, const PopPayoutValue& val) {
        val.toVbkEncoding(stream);
      });
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopPayouts& out,
                                ValidationState& state) {
  size_t i = 0;
  if (!readArrayOf<PopPayoutValue>(
          stream,
          out.values,
          state,
          0,
          MAX_PAYOUT,
          [&i](
              ReadStream& stream, PopPayoutValue& val, ValidationState& state) {
            ++i;
            return DeserializeFromVbkEncoding(stream, val, state);
          })) {
    return state.Invalid("pop-payouts-values", i);
  }

  return true;
}

}  // namespace altintegration