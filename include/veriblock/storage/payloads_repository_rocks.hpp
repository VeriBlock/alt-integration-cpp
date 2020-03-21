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

namespace altintegration {

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Block, typename Payloads>
struct PayloadsCursorRocks
    : public Cursor<typename Block::hash_t,
                    typename PayloadsRepository<Block, Payloads>::
                        stored_payloads_container_t> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! stored payloads container
  using stored_payloads_container_t =
      typename PayloadsRepository<Block, Payloads>::stored_payloads_container_t;

  PayloadsCursorRocks(std::shared_ptr<rocksdb::DB> db,
                      std::shared_ptr<cf_handle_t> payloadsHandle)
      : _db(db) {
    auto iterator =
        _db->NewIterator(rocksdb::ReadOptions(), payloadsHandle.get());
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const hash_t& key) override {
    std::string blockHash(reinterpret_cast<const char*>(key.data()),
                          key.size());
    _iterator->Seek(blockHash);
  }
  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  hash_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto key = _iterator->key();
    hash_t blockHash(key.size());
    for (size_t i = 0; i < blockHash.size(); ++i) {
      blockHash[i] = key[i];
    }

    return blockHash;
  }

  stored_payloads_container_t value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    stored_payloads_container_t payloads_container;

    auto value = _iterator->value();

    ReadStream readStream(value.data(), value.size());
    while (readStream.remaining()) {
      payloads_container.push_back(
          stored_payloads_t::fromVbkEncoding(readStream));
    }

    return payloads_container;
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Block, typename Payloads>
struct PayloadsRepositoryRocks;

template <typename Block, typename Payloads>
struct PayloadsWriteBatchRocks : public PayloadsWriteBatch<Block, Payloads> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  using pair = std::pair<hash_t, stored_payloads_t>;

  enum class Operation { PUT, REMOVE_BY_HASH };

  ~PayloadsWriteBatchRocks() override = default;

  PayloadsWriteBatchRocks(
      PayloadsRepositoryRocks<block_t, stored_payloads_t>* repo)
      : _repo(repo) {}

  void put(const hash_t& hash, const stored_payloads_t& payloads) override {
    _ops.push_back(Operation::PUT);
    _puts.push_back(pair(hash, payloads));
  }

  void removeByHash(const hash_t& hash) override {
    _ops.push_back(Operation::REMOVE_BY_HASH);
    _removes.push_back(hash);
  }

  void clear() override {
    _ops.clear();
    _removes.clear();
    _puts.clear();
  }

  void commit() override {
    auto puts_begin = this->_puts.begin();
    auto removes_begin = this->_removes.begin();
    for (const auto& op : this->_ops) {
      switch (op) {
        case PayloadsWriteBatchRocks<block_t,
                                     stored_payloads_t>::Operation::PUT: {
          _repo->put(puts_begin->first, puts_begin->second);
          ++puts_begin;
          break;
        }
        case PayloadsWriteBatchRocks<block_t, stored_payloads_t>::Operation::
            REMOVE_BY_HASH: {
          _repo->removeByHash(*removes_begin++);
          break;
        }
        default:
          throw std::logic_error(
              "unknown enum value - this should never happen");
      }
    }
    clear();
  }

 private:
  PayloadsRepositoryRocks<block_t, stored_payloads_t>* _repo;
  std::vector<pair> _puts;
  std::vector<hash_t> _removes;
  std::vector<Operation> _ops;
};

template <typename Block, typename Payloads>
struct PayloadsRepositoryRocks : public PayloadsRepository<Block, Payloads> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! stored payloads container type
  using stored_payloads_container_t = std::vector<stored_payloads_t>;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_payloads_container_t>;

  PayloadsRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                          std::shared_ptr<cf_handle_t> payloadsHandle)
      : _db(db), _payloadsHandle(payloadsHandle) {}

  void put(const hash_t& hash, const stored_payloads_t& payloads) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    std::string out{};

    rocksdb::Slice key(reinterpret_cast<const char*>(hash.data()), hash.size());

    s = _db->Get(rocksdb::ReadOptions(), _payloadsHandle.get(), key, &out);

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }

    ReadStream readStream(out.data(), out.size());

    while (readStream.remaining()) {
      stored_payloads_t p = stored_payloads_t::fromVbkEncoding(readStream);
      if (p == payloads) {
        return;
      }
    }

    WriteStream writeStream;
    writeStream.write(out.data(), out.size());
    payloads.toVbkEncoding(writeStream);

    rocksdb::Slice value(
        reinterpret_cast<const char*>(writeStream.data().data()),
        writeStream.data().size());

    s = _db->Put(write_options, _payloadsHandle.get(), key, value);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  stored_payloads_container_t get(const hash_t& hash) const override {
    stored_payloads_container_t payloads_container;

    rocksdb::Status s;

    std::string out{};

    rocksdb::Slice key(reinterpret_cast<const char*>(hash.data()), hash.size());

    s = _db->Get(rocksdb::ReadOptions(), _payloadsHandle.get(), key, &out);

    if (!s.ok()) {
      if (s.IsNotFound()) {
        return {};
      }
      throw db::DbError(s.ToString());
    }

    ReadStream readStream(out.data(), out.size());
    while (readStream.remaining()) {
      payloads_container.push_back(
          stored_payloads_t::fromVbkEncoding(readStream));
    }

    return payloads_container;
  }

  void removeByHash(const hash_t& hash) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    rocksdb::Slice key(reinterpret_cast<const char*>(hash.data()), hash.size());

    s = _db->Delete(write_options, _payloadsHandle.get(), key);

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void clear() override {
    // call BlockRepositoryRocksManager.clear() instead
    return;
  }

  std::unique_ptr<PayloadsWriteBatch<block_t, stored_payloads_t>> newBatch()
      override {
    return std::unique_ptr<PayloadsWriteBatchRocks<block_t, stored_payloads_t>>(
        new PayloadsWriteBatchRocks<block_t, stored_payloads_t>(this));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<PayloadsCursorRocks<block_t, stored_payloads_t>>(
        _db, _payloadsHandle);
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  // [hash_t] -> [set of payloads]
  // endorsement is stored in the raw bytes representation
  std::shared_ptr<cf_handle_t> _payloadsHandle;
};

}  // namespace altintegration

#endif
