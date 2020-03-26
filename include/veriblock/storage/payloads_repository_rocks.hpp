#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "veriblock/alt-util.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/storage/payloads_repository.hpp"
#include "veriblock/storage/rocks_util.hpp"

namespace altintegration {

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Payloads>
struct PayloadsCursorRocks : public Cursor<typename Payloads::id_t, Payloads> {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;
  //! iterator type

  PayloadsCursorRocks(std::shared_ptr<rocksdb::DB> db,
                      std::shared_ptr<cf_handle_t> payloadsHandle)
      : _db(db) {
    auto iterator =
        _db->NewIterator(rocksdb::ReadOptions(), payloadsHandle.get());
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const payloads_id& key) override {
    _iterator->Seek(makeRocksSlice(key));
  }
  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  payloads_id key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return payloads_id(_iterator->key().ToString());
  }

  stored_payloads_t value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    auto value = _iterator->value();

    return stored_payloads_t::fromVbkEncoding(value.ToString());
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Payloads>
struct PayloadsRepositoryRocks;

template <typename Payloads>
struct PayloadsWriteBatchRocks : public PayloadsWriteBatch<Payloads> {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;

  PayloadsWriteBatchRocks(std::shared_ptr<rocksdb::DB> db,
                          std::shared_ptr<cf_handle_t> payloadsHandle)
      : _db(std::move(db)), _payloadsHandle(std::move(payloadsHandle)) {}

  void put(const stored_payloads_t& payloads) override {
    rocksdb::Status s;

    std::string out{};

    auto id = payloads.getId();
    auto payloadsBytes = payloads.toVbkEncoding();

    s = _batch.Put(_payloadsHandle.get(),
                   makeRocksSlice(id),
                   makeRocksSlice(payloadsBytes));

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void removeByHash(const payloads_id& id) override {
    rocksdb::Status s =
        _batch.Delete(_payloadsHandle.get(), makeRocksSlice(id));
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void clear() override { _batch.Clear(); }

  void commit() override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Write(write_options, &_batch);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    clear();
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::shared_ptr<cf_handle_t> _payloadsHandle;
  rocksdb::WriteBatch _batch{};
};

template <typename Payloads>
struct PayloadsRepositoryRocks : public PayloadsRepository<Payloads> {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;
  //! iterator type
  using cursor_t = Cursor<payloads_id, stored_payloads_t>;

  PayloadsRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                          std::shared_ptr<cf_handle_t> payloadsHandle)
      : _db(db), _payloadsHandle(payloadsHandle) {}

  void put(const stored_payloads_t& payloads) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    std::string out{};

    auto id = payloads.getId();
    auto payloadsBytes = payloads.toVbkEncoding();

    s = _db->Put(write_options,
                 _payloadsHandle.get(),
                 makeRocksSlice(id),
                 makeRocksSlice(payloadsBytes));

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  bool get(const payloads_id& id, stored_payloads_t* out) const override {
    rocksdb::Status s;

    std::string valueOut{};

    s = _db->Get(rocksdb::ReadOptions(),
                 _payloadsHandle.get(),
                 makeRocksSlice(id),
                 &valueOut);

    if (!s.ok()) {
      if (s.IsNotFound()) {
        return false;
      }
      throw db::DbError(s.ToString());
    }

    if (out != nullptr) {
      *out = stored_payloads_t::fromVbkEncoding(valueOut);
    }

    return true;
  }

  size_t get(const std::vector<payloads_id>& ids,
             std::vector<stored_payloads_t>* out) const override {
    std::vector<rocksdb::Slice> keys(ids.size());
    std::vector<std::string> valuesOut(ids.size());

    for (size_t i = 0; i < ids.size(); ++i) {
      keys[i] = makeRocksSlice(ids[i]);
    }
    std::vector<cf_handle_t*> cfs(ids.size(), _payloadsHandle.get());
    std::vector<rocksdb::Status> statuses =
        _db->MultiGet(rocksdb::ReadOptions(), cfs, keys, &valuesOut);

    size_t totalFound = 0;
    for (size_t i = 0; i < statuses.size(); ++i) {
      if (!statuses[i].ok()) {
        if (statuses[i].IsNotFound()) {
          continue;
        }
        throw db::DbError(statuses[i].ToString());
      }
      ++totalFound;
      if (out != nullptr) {
        out->push_back(stored_payloads_t::fromVbkEncoding(valuesOut[i]));
      }
    }

    return totalFound;
  }

  void removeByHash(const payloads_id& id) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    s = _db->Delete(write_options, _payloadsHandle.get(), makeRocksSlice(id));

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void clear() override {
    // call BlockRepositoryRocksManager.clear() instead
    return;
  }

  std::unique_ptr<PayloadsWriteBatch<stored_payloads_t>> newBatch() override {
    return std::unique_ptr<PayloadsWriteBatchRocks<stored_payloads_t>>(
        new PayloadsWriteBatchRocks<stored_payloads_t>(_db, _payloadsHandle));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<PayloadsCursorRocks<stored_payloads_t>>(
        _db, _payloadsHandle);
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  // [payloads id] -> [payloads]
  // payloads is stored in the raw bytes representation
  std::shared_ptr<cf_handle_t> _payloadsHandle;
};

}  // namespace altintegration

#endif
