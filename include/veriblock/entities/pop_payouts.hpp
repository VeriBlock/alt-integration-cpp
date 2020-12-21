// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_POP_REWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_POP_REWARDS_HPP_

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

/**
 * @struct PopRewards
 * @ingroup entities
 */
struct PopPayouts {
  using address_t = std::vector<uint8_t>;
  using amount_t = uint64_t;

  std::map<address_t, amount_t> payouts;

  void add(const address_t& address, amount_t amount);

  size_t size() const;

  bool empty() const;

  // explicit cast operator of this struct to std::map
  explicit operator decltype(payouts)() { return payouts; }
  auto begin() -> decltype(payouts.begin()) { return payouts.begin(); }
  auto end() -> decltype(payouts.end()) { return payouts.end(); }
  auto begin() const -> decltype(payouts.begin()) { return payouts.begin(); }
  auto end() const -> decltype(payouts.end()) { return payouts.end(); }

  /**
   * Convert PopRewards to data stream
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         PopPayouts& out,
                                         ValidationState& state);
};

}  // namespace altintegration

#endif