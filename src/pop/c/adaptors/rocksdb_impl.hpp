// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_ROCKSDB_IMPL_HPP
#define VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_ROCKSDB_IMPL_HPP

#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/exceptions/storage_io.hpp>
#include <veriblock/pop/strutil.hpp>

#include "rocksdb/db.h"
#include "storage_interface.hpp"

namespace adaptors {

struct RocksDBStorageIterator : public StorageIterator {
  ~RocksDBStorageIterator() override {
    if (it_ != nullptr) {
      delete it_;
    }
  }

  RocksDBStorageIterator(rocksdb::Iterator* it) : it_(it) {}

  bool value(std::vector<uint8_t>& out) const override;

  bool key(std::vector<uint8_t>& out) const override;

  void next() override { it_->Next(); }

  bool valid() const override { return it_->Valid(); }

  void seek_start() override { it_->SeekToFirst(); }

  void seek(const std::vector<uint8_t>& val) override;

 private:
  rocksdb::Iterator* it_;
};

struct RocksDBWriteBatch : public WriteBatch {
  ~RocksDBWriteBatch() override = default;

  RocksDBWriteBatch(rocksdb::DB& db, rocksdb::WriteOptions& write_options)
      : db_(db), write_options_(write_options) {}

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override;

  void writeBatch() override;

 private:
  rocksdb::DB& db_;
  rocksdb::WriteOptions& write_options_;
  rocksdb::WriteBatch batch_{};
};

struct RocksDBStorage : public Storage {
  ~RocksDBStorage() override;

  RocksDBStorage(const std::string& path);

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override;

  bool read(const std::vector<uint8_t>& key,
            std::vector<uint8_t>& value) override;

  std::shared_ptr<WriteBatch> generateWriteBatch() override {
    return std::make_shared<RocksDBWriteBatch>(*db_, write_options_);
  }

  std::shared_ptr<StorageIterator> generateIterator() override {
    return std::make_shared<RocksDBStorageIterator>(
        db_->NewIterator(read_options_));
  }

 private:
  rocksdb::DB* db_{nullptr};
  rocksdb::WriteOptions write_options_{};
  rocksdb::ReadOptions read_options_{};
};

}  // namespace adaptors

#endif