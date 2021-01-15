// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP
#define VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP

#include <exception>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace altintegration {

/**
 * Is thrown only when this library detects POP-related state corruption, and
 * can't recover.
 */
struct StateCorruptedException : public std::domain_error {
 public:
  StateCorruptedException(const std::string& msg) : std::domain_error(msg) {}

  template <typename Block>
  StateCorruptedException(const BlockIndex<Block>& index,
                          ValidationState& state)
      : StateCorruptedException(
            fmt::format("Error loading payloads from {} block {}, reason={}",
                        Block::name(),
                        index.toPrettyString(),
                        state.toString())) {}
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP
