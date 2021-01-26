// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_ROCKSDB_IMPL_HPP
#define VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_ROCKSDB_IMPL_HPP

#include "rocksdb/db.h"
#include "storage_interface.hpp"
#include "veriblock/assert.hpp"
#include "veriblock/exceptions/storage_io.hpp"
#include "veriblock/strutil.hpp"

namespace adaptors {

struct RocksDBStorageIterator : public StorageIterator {
  ~RocksDBStorageIterator() override { delete it_; };

  RocksDBStorageIterator(rocksdb::Iterator* it) : it_(it) {}

  bool value(std::vector<uint8_t>& out) const override {
    if (!it_->Valid()) {
      return false;
    }
    auto value_slice = it_->value();
    out.resize(value_slice.size());
    for (size_t i = 0; i < value_slice.size(); ++i) {
      out[i] = value_slice[i];
    }
    return true;
  }

  bool key(std::vector<uint8_t>& out) const override {
    if (!it_->Valid()) {
      return false;
    }
    auto key_slice = it_->key();
    out.resize(key_slice.size());
    for (size_t i = 0; i < key_slice.size(); ++i) {
      out[i] = key_slice[i];
    }
    return true;
  }

  void next() override { it_->Next(); }

  bool valid() const override { return it_->Valid(); }

  void seek_start() override { it_->SeekToFirst(); }

  void seek(const std::vector<uint8_t>& val) override {
    rocksdb::Slice val_slice((char*)val.data(), val.size());
    it_->Seek(val_slice);
  }

 private:
  rocksdb::Iterator* it_;
};

struct RocksDBWriteBatch : public WriteBatch {
  ~RocksDBWriteBatch() override = default;

  RocksDBWriteBatch(rocksdb::DB& db, rocksdb::WriteOptions& write_options)
      : db_(db), write_options_(write_options) {}

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override {
    rocksdb::Slice key_slice((char*)key.data(), key.size());
    rocksdb::Slice value_slice((char*)value.data(), value.size());
    rocksdb::Status status = batch_.Put(key_slice, value_slice);
    if (!status.ok()) {
      throw altintegration::StorageIOException(fmt::format(
          "failed to write into the storage, err: %s", status.ToString()));
    }
  }

  void writeBatch() override {
    rocksdb::Status status = db_.Write(write_options_, &batch_);
    if (!status.ok()) {
      throw altintegration::StorageIOException(
          fmt::format("failed to write batch into the storage, err: %s",
                      status.ToString()));
    }
  }

 private:
  rocksdb::DB& db_;
  rocksdb::WriteOptions& write_options_;
  rocksdb::WriteBatch batch_{};
};

struct RocksDBStorage : public Storage {
  ~RocksDBStorage() override { delete db_; }

  RocksDBStorage(const std::string& path) {
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, path, &db_);
    VBK_ASSERT_MSG(status.ok(), status.getState());
  }

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override {
    rocksdb::Slice key_slice((char*)key.data(), key.size());
    rocksdb::Slice value_slice((char*)value.data(), value.size());

    rocksdb::Status status = db_->Put(write_options_, key_slice, value_slice);
    if (!status.ok()) {
      throw altintegration::StorageIOException(fmt::format(
          "failed to write into the storage, err: %s", status.ToString()));
    }
  }

  bool read(const std::vector<uint8_t>& key,
            std::vector<uint8_t>& value) override {
    rocksdb::Slice key_slice((char*)key.data(), key.size());
    std::string str_value;

    rocksdb::Status status = db_->Get(read_options_, key_slice, &str_value);
    value.resize(str_value.size());
    for (size_t i = 0; i < str_value.size(); ++i) {
      value[i] = str_value[i];
    }
    return status.ok();
  }

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