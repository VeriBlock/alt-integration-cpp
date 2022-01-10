// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/storage/adaptors/leveldb_impl.hpp"

namespace altintegration {

namespace adaptors {

void LevelDBWriteBatch::write(const std::vector<uint8_t>& key,
                              const std::vector<uint8_t>& value) {
  leveldb::Slice key_slice((char*)key.data(), key.size());
  leveldb::Slice value_slice((char*)value.data(), value.size());
  batch_.Put(key_slice, value_slice);
}

void LevelDBWriteBatch::writeBatch() {
  leveldb::Status status = db_.Write(write_options_, &batch_);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to write batch into the storage, err: {}", status.ToString()));
  }
}

LevelDBStorage::~LevelDBStorage() {
  if (db_ != nullptr) {
    delete db_;
  }
}

LevelDBStorage::LevelDBStorage(const std::string& path) {
  leveldb::Options options;
  options.create_if_missing = true;

  db_ = nullptr;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to open leveldb storage, err: {}", status.ToString()));
  }
}

void LevelDBStorage::write(const std::vector<uint8_t>& key,
                           const std::vector<uint8_t>& value) {
  leveldb::Slice key_slice((char*)key.data(), key.size());
  leveldb::Slice value_slice((char*)value.data(), value.size());

  leveldb::Status status = db_->Put(write_options_, key_slice, value_slice);
  if (!status.ok()) {
    throw altintegration::StorageIOException(altintegration::format(
        "failed to write into the storage, err: {}", status.ToString()));
  }
}

bool LevelDBStorage::read(const std::vector<uint8_t>& key,
                          std::vector<uint8_t>& value) {
  leveldb::Slice key_slice((char*)key.data(), key.size());
  std::string str_value;

  leveldb::Status status = db_->Get(read_options_, key_slice, &str_value);
  value.resize(str_value.size());
  for (size_t i = 0; i < str_value.size(); ++i) {
    value[i] = str_value[i];
  }
  return status.ok();
}

void LevelDBStorageIterator::seek(const std::vector<uint8_t>& val) {
  leveldb::Slice val_slice((char*)val.data(), val.size());
  it_->Seek(val_slice);
}

bool LevelDBStorageIterator::key(std::vector<uint8_t>& out) const {
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

bool LevelDBStorageIterator::value(std::vector<uint8_t>& out) const {
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