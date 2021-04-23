// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "leveldb_impl.hpp"

adaptors::LevelDBStorage::~LevelDBStorage() {
  if (db_ != nullptr) {
    delete db_;
  }
}

adaptors::LevelDBStorage::LevelDBStorage(const std::string& path) {
  leveldb::Options options;
  options.create_if_missing = true;

  db_ = nullptr;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);
  if (!status.ok()) {
    throw altintegration::StorageIOException(fmt::format(
        "failed to open rocksdb storage, err: {}", status.ToString()));
  }
}

void adaptors::LevelDBStorage::write(const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& value) {
  leveldb::Slice key_slice((char*)key.data(), key.size());
  leveldb::Slice value_slice((char*)value.data(), value.size());

  leveldb::Status status = db_->Put(write_options_, key_slice, value_slice);
  if (!status.ok()) {
    throw altintegration::StorageIOException(fmt::format(
        "failed to write into the storage, err: {}", status.ToString()));
  }
}

bool adaptors::LevelDBStorage::read(const std::vector<uint8_t>& key,
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