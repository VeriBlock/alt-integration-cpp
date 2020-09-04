// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_

namespace altintegration {

enum BlockStatus : uint32_t {
  //! default state for validity - validity state is unknown
  BLOCK_VALID_UNKNOWN = 0,
  //! this is a bootstrap block
  BLOCK_BOOTSTRAP = 1 << 1,
  //! block is statelessly valid, but the altchain marked it as failed
  BLOCK_FAILED_BLOCK = 1 << 2,
  //! block failed state{less,ful} validation due to its payloads
  BLOCK_FAILED_POP = 1 << 3,
  //! block is state{lessly,fully} valid and the altchain did not report it as
  //! invalid, but some of the ancestor blocks are invalid
  BLOCK_FAILED_CHILD = 1 << 4,
  //! all invalidity flags
  BLOCK_FAILED_MASK =
      BLOCK_FAILED_CHILD | BLOCK_FAILED_POP | BLOCK_FAILED_BLOCK,
  //! the block has been applied via PopStateMachine
  BLOCK_APPLIED = 1 << 5,

  //! acceptBlockHeader succeded. All ancestors are at least at this state.
  BLOCK_VALID_TREE = 1 << 6,
  //! acceptBlock has been executed on this block; payloads are statelessly
  //! valid
  BLOCK_HAS_PAYLOADS = 2 << 6,
  //! the block is connected via connectBlock
  BLOCK_CONNECTED = 3 << 6,
  //! the block has been successfully applied, likely along with another chain
  BLOCK_HAS_BEEN_APPLIED = 4 << 6,
  //! the chain with the block at its tip is fully valid
  BLOCK_CAN_BE_APPLIED = 5 << 6,

  //! all validity flags
  BLOCK_VALID_MASK = BLOCK_VALID_UNKNOWN | BLOCK_VALID_TREE |
                     BLOCK_HAS_PAYLOADS | BLOCK_CONNECTED |
                     BLOCK_HAS_BEEN_APPLIED | BLOCK_CAN_BE_APPLIED,

  // DEV NOTE: new flags should be added in the end
};

/**
 * Check if the `reason` value can be used as the reason for invalidateSubtree
 * and revalidateSubtree
 */
constexpr bool isValidInvalidationReason(const enum BlockStatus reason) {
  return reason == BLOCK_FAILED_BLOCK || reason == BLOCK_FAILED_POP;
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_STATUS_HPP_
