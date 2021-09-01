// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP
#define VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP

#include <exception>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>

namespace altintegration {

/**
 * Is thrown only when this library detects POP-related state corruption, and
 * can't recover.
 */
struct StateCorruptedException : public std::domain_error {
  StateCorruptedException(const std::string& msg) : std::domain_error(msg) {}

  template <typename Block>
  StateCorruptedException(const BlockIndex<Block>& index,
                          ValidationState& state)
      : StateCorruptedException(
            format("Error loading payloads from {} block {}, reason={}",
                   Block::name(),
                   index.toPrettyString(),
                   state.toString())) {}
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STATE_CORRUPTED_HPP
