// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_STORAGE_INTERFACE_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_STORAGE_INTERFACE_HPP

#include <memory>
#include <vector>

namespace altintegration {

namespace adaptors {

struct StorageIterator {
  virtual ~StorageIterator() = default;

  virtual bool value(std::vector<uint8_t>& out) const = 0;

  virtual bool key(std::vector<uint8_t>& out) const = 0;

  virtual void next() = 0;

  virtual bool valid() const = 0;

  virtual void seek_start() = 0;

  virtual void seek(const std::vector<uint8_t>& val) = 0;
};

struct WriteBatch {
  virtual ~WriteBatch() = default;

  virtual void write(const std::vector<uint8_t>& key,
                     const std::vector<uint8_t>& value) = 0;

  virtual void writeBatch() = 0;
};

struct Storage {
  virtual ~Storage() = default;

  virtual void write(const std::vector<uint8_t>& key,
                     const std::vector<uint8_t>& value) = 0;

  virtual bool read(const std::vector<uint8_t>& key,
                    std::vector<uint8_t>& out) = 0;

  virtual std::shared_ptr<WriteBatch> generateWriteBatch() = 0;

  virtual std::shared_ptr<StorageIterator> generateIterator() = 0;
};

}  // namespace adaptors

}  // namespace altintegration

#endif
