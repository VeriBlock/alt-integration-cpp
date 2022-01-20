// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/storage/adaptors/rocksdb_impl.hpp"

namespace altintegration {

namespace adaptors {

void RocksDBWriteBatch::write(const std::vector<uint8_t>& key,
                              const std::vector<uint8_t>& value) {
  rocksdb::Slice key_slice((char*)key.data(), key.size());
  rocksdb::Slice value_slice((char*)value.data(), value.size());
  rocksdb::Status status = batch_.Put(key_slice, value_slice);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to write into the storage, err: {}", status.ToString()));
  }
}
void RocksDBWriteBatch::writeBatch() {
  rocksdb::Status status = db_.Write(write_options_, &batch_);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to write batch into the storage, err: {}", status.ToString()));
  }
}
RocksDBStorage::~RocksDBStorage() {
  if (db_ != nullptr) {
    delete db_;
  }
}
RocksDBStorage::RocksDBStorage(const std::string& path) {
  rocksdb::Options options;
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  options.create_if_missing = true;

  db_ = nullptr;
  rocksdb::Status status = rocksdb::DB::Open(options, path, &db_);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to open rocksdb storage, err: {}", status.ToString()));
  }
}
void RocksDBStorage::write(const std::vector<uint8_t>& key,
                           const std::vector<uint8_t>& value) {
  rocksdb::Slice key_slice((char*)key.data(), key.size());
  rocksdb::Slice value_slice((char*)value.data(), value.size());

  rocksdb::Status status = db_->Put(write_options_, key_slice, value_slice);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to write into the storage, err: {}", status.ToString()));
  }
}
bool RocksDBStorage::read(const std::vector<uint8_t>& key,
                          std::vector<uint8_t>& value) {
  rocksdb::Slice key_slice((char*)key.data(), key.size());
  std::string str_value;

  rocksdb::Status status = db_->Get(read_options_, key_slice, &str_value);
  value.resize(str_value.size());
  for (size_t i = 0; i < str_value.size(); ++i) {
    value[i] = str_value[i];
  }
  return status.ok();
}
void RocksDBStorageIterator::seek(const std::vector<uint8_t>& val) {
  rocksdb::Slice val_slice((char*)val.data(), val.size());
  it_->Seek(val_slice);
}
bool RocksDBStorageIterator::key(std::vector<uint8_t>& out) const {
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
bool RocksDBStorageIterator::value(std::vector<uint8_t>& out) const {
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

}  // namespace adaptors

}  // namespace altintegration
