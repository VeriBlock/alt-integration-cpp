// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <veriblock/storage/db_error.hpp>
#include <veriblock/storage/payloads_repository.hpp>
#include <veriblock/storage/rocks/repository_rocks_manager.hpp>
#include <veriblock/storage/rocks/rocks_util.hpp>

namespace altintegration {

template <typename Payloads>
std::vector<uint8_t> serializePayloadsToRocks(const Payloads& from) {
  return from.toVbkEncoding();
}

template <>
inline std::vector<uint8_t> serializePayloadsToRocks(const ATV& from) {
  WriteStream stream;
  from.toVbkEncoding(stream);
  return stream.data();
}

template <>
inline std::vector<uint8_t> serializePayloadsToRocks(const VTB& from) {
  WriteStream stream;
  from.toVbkEncoding(stream);
  return stream.data();
}

template <>
inline std::vector<uint8_t> serializePayloadsToRocks(const VbkBlock& from) {
  WriteStream stream;
  from.toVbkEncoding(stream);
  return stream.data();
}

template <typename Payloads>
Payloads deserializePayloadsFromRocks(const std::string& from) {
  return Payloads::fromVbkEncoding(from);
}

template <>
inline VbkBlock deserializePayloadsFromRocks(const std::string& from) {
  ReadStream stream(from);
  return VbkBlock::fromVbkEncoding(stream);
}

template <>
inline VTB deserializePayloadsFromRocks(const std::string& from) {
  ReadStream stream(from);
  return VTB::fromVbkEncoding(stream);
}

template <>
inline ATV deserializePayloadsFromRocks(const std::string& from) {
  ReadStream stream(from);
  return ATV::fromVbkEncoding(stream);
}

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Payloads>
struct PayloadsCursorRocks : public Cursor<typename Payloads::id_t, Payloads> {
  PayloadsCursorRocks(rocksdb::DB* db, cf_handle_t* columnHandle) : _db(db) {
    auto iterator = _db->NewIterator(rocksdb::ReadOptions(), columnHandle);
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const typename Payloads::id_t& key) override {
    _iterator->Seek(makeRocksSlice(key));
  }

  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  typename Payloads::id_t key() const override {
    VBK_ASSERT(isValid() && "cursor points to an invalid item");
    auto key = _iterator->key().ToString();
    auto keyBytes = std::vector<uint8_t>(key.begin(), key.end());
    return keyBytes;
  }

  Payloads value() const override {
    VBK_ASSERT(isValid() && "cursor points to an invalid item");
    auto value = _iterator->value();
    return deserializePayloadsFromRocks<Payloads>(value.ToString());
  }

 private:
  rocksdb::DB* _db;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Payloads>
struct PayloadsWriteBatchRocks : public PayloadsWriteBatch<Payloads> {
  PayloadsWriteBatchRocks(rocksdb::DB* db, cf_handle_t* columnHandle)
      : _db(db), _columnHandle(columnHandle) {}

  void put(const Payloads& payloads) override {
    auto pid = payloads.getId();
    auto payloadsBytes = serializePayloadsToRocks(payloads);

    rocksdb::Status s = _batch.Put(
        _columnHandle, makeRocksSlice(pid), makeRocksSlice(payloadsBytes));
    if (!s.ok() && !s.IsNotFound()) {
      throw db::StateCorruptedException(s.ToString());
    }
  }

  void remove(const typename Payloads::id_t& pid) override {
    rocksdb::Status s = _batch.Delete(_columnHandle, makeRocksSlice(pid));
    if (!s.ok() && !s.IsNotFound()) {
      throw db::StateCorruptedException(s.ToString());
    }
  }

  void clear() override { _batch.Clear(); }

  void commit() override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Write(write_options, &_batch);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::StateCorruptedException(s.ToString());
    }
    clear();
  }

 private:
  rocksdb::DB* _db;
  cf_handle_t* _columnHandle;
  rocksdb::WriteBatch _batch{};
};

template <typename Payloads>
struct PayloadsRepositoryRocks : public PayloadsRepository<Payloads> {
  using payloads_t = Payloads;
  using pid_t = typename Payloads::id_t;
  using cursor_t = Cursor<pid_t, Payloads>;

  ~PayloadsRepositoryRocks() override = default;

  PayloadsRepositoryRocks() = default;

  PayloadsRepositoryRocks(RepositoryRocksManager& manager,
                          const std::string& name) {
    _columnHandle = manager.getColumn(name);
    _db = manager.getDB();
  }

  bool remove(const pid_t& pid) override {
    std::string value;
    bool existing = _db->KeyMayExist(
        rocksdb::ReadOptions(), _columnHandle, makeRocksSlice(pid), &value);
    if (!existing) return false;

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s =
        _db->Delete(write_options, _columnHandle, makeRocksSlice(pid));
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::StateCorruptedException(s.ToString());
    }
    return true;
  }

  bool put(const payloads_t& payload) override {
    auto pid = payload.getId();
    std::string value;
    bool existing = _db->KeyMayExist(
        rocksdb::ReadOptions(), _columnHandle, makeRocksSlice(pid), &value);
    auto payloadsBytes = serializePayloadsToRocks(payload);

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Put(write_options,
                                 _columnHandle,
                                 makeRocksSlice(pid),
                                 makeRocksSlice(payloadsBytes));
    if (!s.ok()) {
      throw db::StateCorruptedException(s.ToString());
    }
    return existing;
  }

  bool get(const pid_t& pid, payloads_t* out) const override {
    std::string dbValue{};
    rocksdb::Status s = _db->Get(
        rocksdb::ReadOptions(), _columnHandle, makeRocksSlice(pid), &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::StateCorruptedException(s.ToString());
    }

    *out = deserializePayloadsFromRocks<payloads_t>(dbValue);
    return true;
  }

  void clear() override {
    auto cursor = newCursor();
    VBK_ASSERT(cursor && "can not create cursor");
    cursor->seekToFirst();
    while (cursor->isValid()) {
      auto key = cursor->key();
      remove(key);
      cursor->next();
    }
  }

  std::unique_ptr<PayloadsWriteBatch<Payloads>> newBatch() override {
    return std::unique_ptr<PayloadsWriteBatchRocks<Payloads>>(
        new PayloadsWriteBatchRocks<Payloads>(_db, _columnHandle));
  }

  std::shared_ptr<cursor_t> newCursor() const override {
    return std::make_shared<PayloadsCursorRocks<Payloads>>(_db, _columnHandle);
  }

 private:
  rocksdb::DB* _db;
  cf_handle_t* _columnHandle;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_REPOSITORY_ROCKS_HPP_
