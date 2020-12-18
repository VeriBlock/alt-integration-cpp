// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/pop_rewards.hpp"

namespace altintegration {

void PopRewards::toVbkEncoding(WriteStream& stream) const {
  writeArrayOf<payout_value_t>(
      stream, this->payout, [](WriteStream& stream, const payout_value_t& val) {
        writeSingleByteLenValue(stream, val.first);
        stream.writeBE<int64_t>(val.second);
      });
}

size_t PopRewards::estimateSize() const { return 0; }

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopRewards& out,
                                ValidationState& state) {
  size_t i = 0;
  if (!readArrayOf<typename PopRewards::payout_value_t>(
          stream,
          out.payout,
          state,
          0,
          MAX_PAYOUT,
          [&i](ReadStream& stream,
               typename PopRewards::payout_value_t& val,
               ValidationState& state) {
            ++i;
            return readSingleByteLenValue<std::vector<uint8_t>>(
                stream, val.first, state, 0);
          })) {
    return state.Invalid("payouts-value", i);
  }

  return true;
}

}  // namespace altintegration