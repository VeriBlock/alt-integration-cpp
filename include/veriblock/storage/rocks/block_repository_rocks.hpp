// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>
#include <veriblock/serde.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/storage/db_error.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/storage/rocks/rocks_util.hpp>
#include <veriblock/storage/rocks/repository_rocks_manager.hpp>

namespace altintegration {

template <typename Block>
std::vector<uint8_t> serializeBlockToRocks(const Block&);

template <>
inline std::vector<uint8_t> serializeBlockToRocks(const BlockIndex<BtcBlock>& from) {
  WriteStream s;
  from.toRaw(s);
  s.writeBE<uint32_t>((uint32_t)from.refCounter);
  s.writeBE<uint8_t>((uint8_t)from.status);
  return s.data();
}

template <>
inline std::vector<uint8_t> serializeBlockToRocks(
    const BlockIndex<VbkBlock>& from) {
  WriteStream s;
  from.toRaw(s);
  s.writeBE<uint32_t>((uint32_t)from.containingEndorsements.size());
  for (const auto& e : from.containingEndorsements) {
    writeSingleByteLenValue(s, e.first);
  }
  s.writeBE<uint32_t>((uint32_t)from.vtbids.size());
  for (const auto& p : from.vtbids) {
    writeSingleByteLenValue(s, p);
  }
  s.writeBE<uint32_t>((uint32_t)from.refCounter);
  s.writeBE<uint8_t>((uint8_t)from.status);
  return s.data();
}

template <>
inline std::vector<uint8_t> serializeBlockToRocks(
    const BlockIndex<AltBlock>& from) {
  WriteStream s;
  from.toRaw(s);
  s.writeBE<uint32_t>((uint32_t)from.containingEndorsements.size());
  for (const auto& e : from.containingEndorsements) {
    writeSingleByteLenValue(s, e.first);
  }
  s.writeBE<uint32_t>((uint32_t)from.atvids.size());
  for (const auto& p : from.atvids) {
    writeSingleByteLenValue(s, p);
  }
  s.writeBE<uint32_t>((uint32_t)from.vtbids.size());
  for (const auto& p : from.vtbids) {
    writeSingleByteLenValue(s, p);
  }
  s.writeBE<uint32_t>((uint32_t)from.vbkblockids.size());
  for (const auto& p : from.vbkblockids) {
    writeSingleByteLenValue(s, p);
  }
  s.writeBE<uint8_t>((uint8_t)from.status);
  return s.data();
}

template <typename Block>
Block deserializeBlockFromRocks(const std::string&);

template <>
inline BlockIndex<BtcBlock> deserializeBlockFromRocks(const std::string& from) {
  ReadStream stream(from);
  auto block = BlockIndex<BtcBlock>::fromRaw(stream);
  block.refCounter = stream.readBE<uint32_t>();
  block.status = stream.readBE<uint8_t>();
  return block;
}

template <>
inline BlockIndex<VbkBlock> deserializeBlockFromRocks(const std::string& from) {
  ReadStream stream(from);
  auto block = BlockIndex<VbkBlock>::fromRaw(stream);
  auto endorsementSize = stream.readBE<uint32_t>();
  for (uint32_t i = 0; i < endorsementSize; ++i) {
    auto eid = readSingleByteLenValue(stream);
    block.containingEndorsements.insert({eid, {}});
  }
  auto payloadSize = stream.readBE<uint32_t>();
  block.vtbids.resize(payloadSize);
  for (uint32_t i = 0; i < payloadSize; ++i) {
    auto pid = readSingleByteLenValue(stream);
    block.vtbids[i] = pid;
  }
  block.refCounter = stream.readBE<uint32_t>();
  block.status = stream.readBE<uint8_t>();
  return block;
}

template <>
inline BlockIndex<AltBlock> deserializeBlockFromRocks(const std::string& from) {
  ReadStream stream(from);
  auto block = BlockIndex<AltBlock>::fromRaw(stream);
  auto endorsementSize = stream.readBE<uint32_t>();
  for (uint32_t i = 0; i < endorsementSize; ++i) {
    auto eid = readSingleByteLenValue(stream);
    block.containingEndorsements.insert({eid, {}});
  }
  auto payloadSize = stream.readBE<uint32_t>();
  block.atvids.resize(payloadSize);
  for (uint32_t i = 0; i < payloadSize; ++i) {
    auto pid = readSingleByteLenValue(stream);
    block.atvids[i] = pid;
  }
  payloadSize = stream.readBE<uint32_t>();
  block.vtbids.resize(payloadSize);
  for (uint32_t i = 0; i < payloadSize; ++i) {
    auto pid = readSingleByteLenValue(stream);
    block.vtbids[i] = pid;
  }
  payloadSize = stream.readBE<uint32_t>();
  block.vbkblockids.resize(payloadSize);
  for (uint32_t i = 0; i < payloadSize; ++i) {
    auto pid = readSingleByteLenValue(stream);
    block.vbkblockids[i] = pid;
  }
  block.status = stream.readBE<uint8_t>();
  return block;
}

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Block>
struct BlockCursorRocks : public Cursor<typename Block::hash_t, Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;

  BlockCursorRocks(rocksdb::DB* db, cf_handle_t* columnHandle)
      : _db(db) {
    auto iterator = _db->NewIterator(rocksdb::ReadOptions(), columnHandle);
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const hash_t& key) override {
    _iterator->Seek(makeRocksSlice(key));
  }
  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  hash_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto key = _iterator->key().ToString();
    auto keyBytes = std::vector<uint8_t>(key.begin(), key.end());
    return keyBytes;
  }

  stored_block_t value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto value = _iterator->value();
    return deserializeBlockFromRocks<stored_block_t>(value.ToString());
  }

 private:
  rocksdb::DB* _db;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Block>
struct BlockWriteBatchRocks : public BlockWriteBatch<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  BlockWriteBatchRocks(rocksdb::DB* db, cf_handle_t* columnHandle)
      : _db(db), _columnHandle(columnHandle) {}

  void put(const stored_block_t& block) override {
    auto blockHash = block.getHash();
    auto blockBytes = serializeBlockToRocks(block);

    rocksdb::Status s = _batch.Put(
        _columnHandle, makeRocksSlice(blockHash), makeRocksSlice(blockBytes));
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void removeByHash(const hash_t& hash) override {
    rocksdb::Status s = _batch.Delete(_columnHandle, makeRocksSlice(hash));
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
  rocksdb::DB* _db;
  cf_handle_t* _columnHandle;
  rocksdb::WriteBatch _batch{};
};

template <typename Block>
class BlockRepositoryRocks : public BlockRepository<Block> {
 public:
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

 public:
  BlockRepositoryRocks() = default;

  BlockRepositoryRocks(RepositoryRocksManager& manager, const std::string& name) {
    _columnHandle = manager.getColumn(name);
    _db = manager.getDB();
  }

  bool put(const stored_block_t& block) override {
    auto hash = block.getHash();
    auto hashSlice = Slice<uint8_t>(hash.data(), hash.size());
    auto key = makeRocksSlice(hashSlice);
    std::string value;
    bool existing = _db->KeyMayExist(rocksdb::ReadOptions(),
                                     _columnHandle, key, &value);

    auto blockBytes = serializeBlockToRocks(block);

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Put(write_options,
                                 _columnHandle, key,
                                 makeRocksSlice(blockBytes));
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
    return existing;
  }

  bool getByHash(const hash_t& hash, stored_block_t* out) const override {
    std::string dbValue{};
    rocksdb::Status s = _db->Get(
        rocksdb::ReadOptions(), _columnHandle, makeRocksSlice(hash), &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::DbError(s.ToString());
    }

    *out = deserializeBlockFromRocks<stored_block_t>(dbValue);
    return true;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) const override {
    size_t numKeys = hashes.size();
    std::vector<rocksdb::Slice> keys(numKeys);
    size_t i = 0;
    for (const auto& h : hashes) {
      keys[i++] = makeRocksSlice(h);
    }
    std::vector<rocksdb::PinnableSlice> values(numKeys);
    std::vector<rocksdb::Status> statuses(numKeys);
    _db->MultiGet(rocksdb::ReadOptions(),
                  _columnHandle,
                  numKeys,
                  keys.data(),
                  values.data(),
                  statuses.data());

    size_t found = 0;
    for (i = 0; i < numKeys; i++) {
      auto& status = statuses[i];
      if (status.ok()) {
        out->push_back(deserializeBlockFromRocks<stored_block_t>(values[i].ToString()));
        found++;
        continue;
      }
      if (status.IsNotFound()) continue;
      throw db::DbError(status.ToString());
    }
    return found;
  }

  bool removeByHash(const hash_t& hash) override {
    auto key = makeRocksSlice(hash);
    std::string value;
    bool existing = _db->KeyMayExist(rocksdb::ReadOptions(), _columnHandle, key, &value);
    if (!existing) return false;

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Delete(write_options, _columnHandle, key);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  void clear() override {
    auto cursor = newCursor();
    if (cursor == nullptr) {
      throw db::DbError("Cannot create BlockRepository cursor");
    }
    cursor->seekToFirst();
    while (cursor->isValid()) {
      auto key = cursor->key();
      removeByHash(key);
      cursor->next();
    }
  }

  std::unique_ptr<BlockWriteBatch<stored_block_t>> newBatch() override {
    return std::unique_ptr<BlockWriteBatchRocks<stored_block_t>>(
        new BlockWriteBatchRocks<stored_block_t>(_db, _columnHandle));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<BlockCursorRocks<Block>>(_db, _columnHandle);
  }

 private:
  rocksdb::DB* _db;
  cf_handle_t* _columnHandle;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_BLOCK_REPOSITORY_ROCKS_HPP_
