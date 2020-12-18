// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_POP_REWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_POP_REWARDS_HPP_

#include <cstdint>
#include <utility>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

/**
 * @struct PopPayoutValue
 * @ingroup entities
 */
struct PopPayoutValue {
  std::vector<uint8_t> address;
  uint64_t amount;

  /**
   * Convert PopPayoutValue to data stream
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  friend bool operator==(const PopPayoutValue& a, const PopPayoutValue& b);
  friend bool operator!=(const PopPayoutValue& a, const PopPayoutValue& b);
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopPayoutValue& out,
                                ValidationState& state);

/**
 * @struct PopRewards
 * @ingroup entities
 */
struct PopPayouts {
  std::vector<PopPayoutValue> values{};

  void add(const PopPayoutValue& value);

  size_t size() const;

  bool empty() const;

  std::vector<PopPayoutValue> find_payouts(
      const std::vector<uint8_t>& address) const;

  uint64_t amount_for_address(const std::vector<uint8_t>& address) const;

  /**
   * Convert PopRewards to data stream
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopPayouts& out,
                                ValidationState& state);

}  // namespace altintegration

#endif