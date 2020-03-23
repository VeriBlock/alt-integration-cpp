#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENTS_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENTS_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <memory>
#include <string>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/storage/endorsement_repository.hpp"

namespace altintegration {

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Endorsement>
struct EndorsementCursorRocks
    : public Cursor<typename Endorsement::id_t, Endorsement> {
  using eid_t = typename Endorsement::id_t;

  EndorsementCursorRocks(std::shared_ptr<rocksdb::DB> db,
                         std::shared_ptr<cf_handle_t> endorsedIdHandle)
      : _db(db) {
    auto iterator =
        _db->NewIterator(rocksdb::ReadOptions(), endorsedIdHandle.get());
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const eid_t& key) override {
    rocksdb::Slice eid(reinterpret_cast<const char*>(key.data()), key.size());
    _iterator->Seek(eid);
  }
  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  eid_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto key = _iterator->key();

    return eid_t(key.ToString());
  }

  Endorsement value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto value = _iterator->value();
    std::string str = value.ToString();

    return Endorsement::fromVbkEncoding(value.ToString());
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Endorsement>
struct EndorsementWriteBatchRocks : public EndorsementWriteBatch<Endorsement> {
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;

  EndorsementWriteBatchRocks(std::shared_ptr<rocksdb::DB> db,
                             std::shared_ptr<cf_handle_t> endorsedIdHandle)
      : _db(std::move(db)), _endorsedIdHandle(std::move(endorsedIdHandle)) {}

  void put(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);
    put(e);
  }

  void put(const endorsement_t& e) override {
    rocksdb::Status s;

    std::vector<uint8_t> bytes = e.toVbkEncoding();
    rocksdb::Slice key(reinterpret_cast<const char*>(e.id.data()), e.id.size());
    rocksdb::Slice val(reinterpret_cast<const char*>(bytes.data()),
                       bytes.size());

    s = _batch.Put(_endorsedIdHandle.get(), key, val);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  void remove(const eid_t& e_id) override {
    rocksdb::Status s;

    rocksdb::Slice key(reinterpret_cast<const char*>(e_id.data()), e_id.size());
    s = _batch.Delete(_endorsedIdHandle.get(), key);

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void remove(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);
    remove(e.id);
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
  std::shared_ptr<cf_handle_t> _endorsedIdHandle;
  rocksdb::WriteBatch _batch{};
};

template <typename Endorsement>
class EndorsementRepositoryRocks : public EndorsementRepository<Endorsement> {
 public:
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;
  using cursor_t = Cursor<eid_t, Endorsement>;

 public:
  EndorsementRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                             std::shared_ptr<cf_handle_t> endorsedIdHandle)
      : _db(std::move(db)), _endorsedIdHandle(std::move(endorsedIdHandle)) {}

  void remove(const eid_t& e_id) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    rocksdb::Slice key(reinterpret_cast<const char*>(e_id.data()), e_id.size());
    s = _db->Delete(write_options, _endorsedIdHandle.get(), key);

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void remove(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);
    remove(e.id);
  }

  void put(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);
    put(e);
  }

  void put(const endorsement_t& e) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    std::vector<uint8_t> bytes = e.toVbkEncoding();
    rocksdb::Slice key(reinterpret_cast<const char*>(e.id.data()), e.id.size());
    rocksdb::Slice val(reinterpret_cast<const char*>(bytes.data()),
                       bytes.size());

    s = _db->Put(write_options, _endorsedIdHandle.get(), key, val);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  std::vector<endorsement_t> get(
      const endorsed_hash_t& endorsedBlockHash) const override {
    std::vector<endorsement_t> endorsements;
    auto cursor = newCursor();

    for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
      endorsement_t e = cursor->value();

      if (e.endorsedHash == endorsedBlockHash) {
        endorsements.push_back(e);
      }
    }

    return endorsements;
  }

  std::unique_ptr<EndorsementWriteBatch<Endorsement>> newBatch() override {
    return std::unique_ptr<EndorsementWriteBatchRocks<Endorsement>>(
        new EndorsementWriteBatchRocks<Endorsement>(_db, _endorsedIdHandle));
  }

  std::shared_ptr<cursor_t> newCursor() const override {
    return std::make_shared<EndorsementCursorRocks<Endorsement>>(
        _db, _endorsedIdHandle);
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  // [endorsement id] -> [endorsement]
  // endorsement is stored in the raw bytes representation
  std::shared_ptr<cf_handle_t> _endorsedIdHandle;
};

}  // namespace altintegration

#endif
