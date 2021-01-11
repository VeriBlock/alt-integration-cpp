// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ITERATOR_HPP

#include <stdint.h>

#include <vector>

/**
 * @defgroup interfaces Interfaces to be implemented
 * These interfaces must be implemented by Altchain developers for integration
 * of POP protocol.
 */

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
struct BlockHashIterator {
  virtual ~BlockHashIterator() = default;

  virtual bool next() = 0;

  virtual std::vector<uint8_t> value() const = 0;

  virtual bool valid() const = 0;

  virtual bool seek_start() = 0;
};

}  // namespace altintegration

#endif