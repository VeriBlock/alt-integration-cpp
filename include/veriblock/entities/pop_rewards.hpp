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
 * @struct PopRewards
 * @ingroup entities
 */
struct PopRewards {
  using payout_value_t = std::pair<std::vector<uint8_t>, int64_t>;
  using payots_container_t = std::vector<payout_value_t>;

  payots_container_t payout;

  /**
   * Convert PopRewards to data stream using PublicationData byte
   * format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopRewards& out,
                                ValidationState& state);

}  // namespace altintegration

#endif