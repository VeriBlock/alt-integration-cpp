// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_LEVELDB_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_LEVELDB_IMPL_HPP

#include "leveldb/db.h"
#include "leveldb/iterator.h"
#include "leveldb/write_batch.h"
#include "storage_interface.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/exceptions/storage_io.hpp"
#include "veriblock/pop/strutil.hpp"

namespace altintegration {

namespace adaptors {

struct LevelDBStorageIterator : public StorageIterator {
  ~LevelDBStorageIterator() override {
    if (it_ != nullptr) {
      delete it_;
    }
  }

  LevelDBStorageIterator(leveldb::Iterator* it) : it_(it) {}

  bool value(std::vector<uint8_t>& out) const override;

  bool key(std::vector<uint8_t>& out) const override;

  void next() override { it_->Next(); }

  bool valid() const override { return it_->Valid(); }

  void seek_start() override { it_->SeekToFirst(); }

  void seek(const std::vector<uint8_t>& val) override;

 private:
  leveldb::Iterator* it_;
};

struct LevelDBWriteBatch : public WriteBatch {
  ~LevelDBWriteBatch() override = default;

  LevelDBWriteBatch(leveldb::DB& db, leveldb::WriteOptions& write_options)
      : db_(db), write_options_(write_options) {}

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override;

  void writeBatch() override;

 private:
  leveldb::DB& db_;
  leveldb::WriteOptions& write_options_;
  leveldb::WriteBatch batch_{};
};

struct LevelDBStorage : public Storage {
  ~LevelDBStorage() override;

  LevelDBStorage(const std::string& path);

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override;

  bool read(const std::vector<uint8_t>& key,
            std::vector<uint8_t>& value) override;

  std::shared_ptr<WriteBatch> generateWriteBatch() override {
    return std::make_shared<LevelDBWriteBatch>(*db_, write_options_);
  }

  std::shared_ptr<StorageIterator> generateIterator() override {
    return std::make_shared<LevelDBStorageIterator>(
        db_->NewIterator(read_options_));
  }

 private:
  leveldb::DB* db_{nullptr};
  leveldb::WriteOptions write_options_{};
  leveldb::ReadOptions read_options_{};
};

}  // namespace adaptors

}  // namespace altintegration

#endif