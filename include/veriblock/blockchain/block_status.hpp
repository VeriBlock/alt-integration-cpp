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
  //! the block is currently applied via SetState.
  BLOCK_ACTIVE = 1 << 5,

  //! acceptBlockHeader succeded. All ancestors are at least at this state.
  BLOCK_VALID_TREE = 1 << 6,
  //! acceptBlock has been executed on this block; payloads are at least
  //! statelessly valid
  BLOCK_HAS_PAYLOADS = 2 << 6,
  //! the block is connected via connectBlock, which means that this block and
  //! all ancestors are at least "BLOCK_HAS_PAYLOADS"
  BLOCK_CONNECTED = 3 << 6,
  //! the block has been successfully applied, but may not be fully valid,
  //! because it may connect to the "other" chain when two chains are applied
  //! together during POP FR
  BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN = 4 << 6,
  //! the chain with the block at its tip is fully valid, so if we do SetState
  //! on this block, it is guaranteed to succeed.
  BLOCK_CAN_BE_APPLIED = 5 << 6,

  //! all stateful validity levels
  // FIXME: BLOCK_HAS_PAYLOADS is not really a stateful validity level and does
  // not belong here since it does not depend on other block contents
  BLOCK_VALID_MASK = BLOCK_VALID_UNKNOWN | BLOCK_VALID_TREE |
                     BLOCK_HAS_PAYLOADS | BLOCK_CONNECTED |
                     BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN |
                     BLOCK_CAN_BE_APPLIED,

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
