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
 * @struct StorageIterator
 *
 * An abstraction over on-disk storage iterator.
 *
 * veriblock-pop-cpp does not dictate how to iterate over the storage. Altchain
 * must create derived class.
 *
 * @ingroup interfaces
 */

template <typename BlockT>
struct BlockIterator {
  using hash_t = typename BlockT::hash_t;

  virtual ~BlockIterator() = default;

  virtual void next() = 0;

  virtual BlockIndex<BlockT> value() const = 0;

  virtual hash_t key() const = 0;

  virtual bool valid() const = 0;

  virtual bool seek_start() = 0;
};

}  // namespace altintegration

#endif