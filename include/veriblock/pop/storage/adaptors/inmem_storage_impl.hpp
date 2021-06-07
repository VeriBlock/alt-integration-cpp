// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_INMEM_STORAGE_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_INMEM_STORAGE_IMPL_HPP

#include <map>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/exceptions/storage_io.hpp>
#include <veriblock/pop/strutil.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

struct InmemStorageIterator : public StorageIterator {
  ~InmemStorageIterator() override = default;

  InmemStorageIterator(
      std::map<std::vector<uint8_t>, std::vector<uint8_t>>& storage)
      : storage_(storage), it_(storage.begin()) {}

  bool value(std::vector<uint8_t>& out) const override {
    out = it_->second;
    return true;
  }

  bool key(std::vector<uint8_t>& out) const override {
    out = it_->first;
    return true;
  }

  void next() override { ++it_; }

  bool valid() const override { return it_ != storage_.end(); }

  void seek_start() override { it_ = storage_.begin(); }

  void seek(const std::vector<uint8_t>& val) override {
    it_ = storage_.begin();
    for (; it_ != storage_.end(); ++it_) {
      if (is_sub_vec(it_->first, val)) {
        break;
      }
    }
  }

 private:
  bool is_sub_vec(const std::vector<uint8_t>& cur,
                  const std::vector<uint8_t>& sub) {
    if (sub.size() > cur.size()) {
      return false;
    }
    for (size_t i = 0; i < sub.size(); ++i) {
      if (cur[i] != sub[i]) {
        return false;
      }
    }
    return true;
  }

  std::map<std::vector<uint8_t>, std::vector<uint8_t>>& storage_;
  std::map<std::vector<uint8_t>, std::vector<uint8_t>>::iterator it_;
};

struct InmemWriteBatch : public WriteBatch {
  ~InmemWriteBatch() override = default;

  InmemWriteBatch(std::map<std::vector<uint8_t>, std::vector<uint8_t>>& storage)
      : storage_(storage) {}

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override {
    storage_[key] = value;
  }

  void writeBatch() override {}

 private:
  std::map<std::vector<uint8_t>, std::vector<uint8_t>>& storage_;
};

struct InmemStorageImpl : public Storage {
  ~InmemStorageImpl() override = default;

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override {
    storage_[key] = value;
  }

  bool read(const std::vector<uint8_t>& key,
            std::vector<uint8_t>& value) override {
    const auto& it = storage_.find(key);
    if (it != storage_.end()) {
      value = it->second;
      return true;
    }
    return false;
  }

  std::shared_ptr<WriteBatch> generateWriteBatch() override {
    return std::make_shared<InmemWriteBatch>(storage_);
  }

  std::shared_ptr<StorageIterator> generateIterator() override {
    return std::make_shared<InmemStorageIterator>(storage_);
  }

 private:
  std::map<std::vector<uint8_t>, std::vector<uint8_t>> storage_;
};

}  // namespace adaptors

}  // namespace altintegration

#endif
