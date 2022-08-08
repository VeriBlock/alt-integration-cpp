// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/pop_payouts.hpp>
#include <utility>


#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

namespace altintegration {

void PopPayouts::toVbkEncoding(WriteStream& stream) const {
  writeContainer<std::map<payout_info_t, amount_t>>(
      stream,
      payouts,
      [](WriteStream& stream, const std::pair<payout_info_t, amount_t>& value) {
        writeSingleByteLenValue(stream, value.first);
        stream.writeBE<uint64_t>(value.second);
      });
}

void PopPayouts::add(const payout_info_t& address, amount_t amount) {
  this->payouts[address] += amount;
}

size_t PopPayouts::size() const { return this->payouts.size(); }

bool PopPayouts::empty() const { return this->payouts.empty(); }

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopPayouts& out,
                                ValidationState& state) {
  using payout_info_t = typename PopPayouts::payout_info_t;
  using amount_t = typename PopPayouts::amount_t;

  std::vector<std::pair<payout_info_t, amount_t>> out_vec;
  size_t i = 0;
  if (!readArrayOf<std::pair<payout_info_t, amount_t>>(
          stream,
          out_vec,
          state,
          0,
          MAX_PAYOUT,
          [&i](ReadStream& stream,
               std::pair<payout_info_t, amount_t>& val,
               ValidationState& state) {
            ++i;

            if (!readSingleByteLenValue<std::vector<uint8_t>>(
                    stream, val.first, state, 0, MAX_PAYOUT_INFO_SIZE)) {
              return state.Invalid("address");
            }

            if (!stream.readBE<uint64_t>(val.second, state)) {
              return state.Invalid("amount");
            }

            return true;
          })) {
    return state.Invalid("pop-payouts-values", i);
  }

  for (const auto& el : out_vec) {
    out.payouts[el.first] += el.second;
  }

  return true;
}

}  // namespace altintegration
