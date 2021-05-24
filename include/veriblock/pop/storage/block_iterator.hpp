// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP

#include <stdint.h>

#include <vector>

namespace altintegration {

/**
 * @struct BlockIterator
 *
 * An abstraction over on-disk storage block iterator.
 *
 * veriblock-pop-cpp does not dictate how to iterate blocks over the storage.
 * Altchain must create derived class.
 *
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
   * @param[out] out block, altintegration::StoredBlockIndex
   * @return true if we can successfully return value, false otherwise
   */
  virtual bool value(StoredBlockIndex<BlockT>& out) const = 0;

  /**
   * Return the current block hash key
   * @param[out] out block hash
   * @return true if we can successfully return key, false otherwise
   */
  virtual bool key(hash_t& out) const = 0;

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