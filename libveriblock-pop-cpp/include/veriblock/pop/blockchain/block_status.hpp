// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_

#include <stdint.h>

namespace altintegration {

//! Flags that describe block status.
enum BlockStateStatus : uint32_t {
  //! acceptBlockHeader succeded. All ancestors are at least at this state and
  //! statelessly valid.
  BLOCK_VALID_TREE = 1,
  //! the block is connected via connectBlock, which means that this block and
  //! all its ancestors are at least BLOCK_CONNECTED and have BLOCK_HAS_PAYLOADS
  //! set.
  BLOCK_CONNECTED = 2,
  //! the block has been successfully applied, but may not be fully valid,
  //! because it may connect to the "other" chain when two chains are applied
  //! together during POP FR. All the ancestors are at least
  //! BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN.
  BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN = 3,
  //! the chain with the block at its tip is fully valid, so if we do SetState
  //! on this block, it is guaranteed to succeed. All the ancestors are at least
  //! BLOCK_CAN_BE_APPLIED.
  BLOCK_CAN_BE_APPLIED = 4,

  //! all stateful validity levels
  BLOCK_VALID_MASK = BLOCK_VALID_TREE | BLOCK_CONNECTED |
                     BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN |
                     BLOCK_CAN_BE_APPLIED,

};

//! Flags that describe block status.
enum BlockValidityStatus : uint32_t {
  //! default state for validity - validity state is unknown
  BLOCK_VALID_UNKNOWN = 0,

  // all values from (0, 15] are reserved for BlockStateStatus

  //! this is a bootstrap block
  BLOCK_BOOTSTRAP = 1 << 4,
  //! block is statelessly valid, but the altchain marked it as failed
  BLOCK_FAILED_BLOCK = 1 << 5,
  //! block failed state{less,ful} validation due to its payloads
  BLOCK_FAILED_POP = 1 << 6,
  //! block is state{lessly,fully} valid and the altchain did not report it as
  //! invalid, but some of the ancestor blocks are invalid
  BLOCK_FAILED_CHILD = 1 << 7,
  //! all invalidity flags
  BLOCK_FAILED_MASK =
      BLOCK_FAILED_CHILD | BLOCK_FAILED_POP | BLOCK_FAILED_BLOCK,
  //! acceptBlockHeader has been executed on this block;
  BLOCK_HAS_PAYLOADS = 1 << 8,
  //! the block is currently applied via SetState.
  BLOCK_ACTIVE = 1 << 9,
  //! the block is temporarily deleted
  BLOCK_DELETED = 1 << 10

};

/**
 * Check if the `reason` value can be used as the reason for invalidateSubtree
 * and revalidateSubtree
 */
constexpr bool isValidInvalidationReason(const BlockValidityStatus& reason) {
  return reason == BLOCK_FAILED_BLOCK || reason == BLOCK_FAILED_POP;
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_
