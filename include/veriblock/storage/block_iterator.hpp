// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP

#include <stdint.h>

#include <vector>

#include "veriblock/blockchain/block_index.hpp"

namespace altintegration {

/**
 * @struct BlockIterator
 *
 * An abstraction over on-disk storage block iterator.
 *
 * veriblock-pop-cpp does not dictate how to iterate blocks over the storage.
 * Altchain must create derived class.
 *
 * @ingroup interfaces
 */

template <typename BlockT>
struct BlockIterator {
  using hash_t = typename BlockT::hash_t;

  virtual ~BlockIterator() = default;

  /**
   * Iterate to the next block.
   */
  virtual void next() = 0;

  /**
   * Return the current block value
   * @return current block, altintegration::BlockIndex
   */
  virtual BlockIndex<BlockT> value() const = 0;

  /**
   * Return the current block hash key
   * @return current block hash
   */
  virtual hash_t key() const = 0;

  /**
   * Return if valid iterator or not
   * @return true if the iterator is valid, false otherwise
   */
  virtual bool valid() const = 0;

  /**
   * Set iterator to the start of the collection
   */
  virtual void seek_start() = 0;
};

}  // namespace altintegration

#endif