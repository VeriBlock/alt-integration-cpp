// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP
#define VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP

#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/storage/stored_block_index.hpp>

namespace altintegration {

/**
 * An interface which represents single write batch of a group of blocks.
 *
 * Altchains are expected to implement this interface to allow alt-cpp saving
 * blocks on disk.
 */
struct BlockBatch {
  virtual ~BlockBatch() = default;

  /**
   * Save BlockIndex on disk.
   * @param value
   */
  virtual void writeBlock(const StoredBlockIndex<AltBlock>& value) = 0;
  //! @overload
  virtual void writeBlock(const StoredBlockIndex<VbkBlock>& value) = 0;
  //! @overload
  virtual void writeBlock(const StoredBlockIndex<BtcBlock>& value) = 0;

  /**
   * Save Tree tip on disk.
   * @param value
   */
  virtual void writeTip(const StoredBlockIndex<AltBlock>& value) = 0;
  //! @overload
  virtual void writeTip(const StoredBlockIndex<VbkBlock>& value) = 0;
  //! @overload
  virtual void writeTip(const StoredBlockIndex<BtcBlock>& value) = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP
