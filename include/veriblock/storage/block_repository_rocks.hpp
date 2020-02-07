#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <set>

#include "veriblock/serde.hpp"
#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

template <typename Block>
class BlockRepositoryRocks : public BlockRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  //! column family type
  using cf_handle_t = rocksdb::ColumnFamilyHandle;

 public:
  BlockRepositoryRocks() = default;

  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                       std::shared_ptr<cf_handle_t> heightHashesHandle,
                       std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(db),
        _heightHashesHandle(heightHashesHandle),
        _hashBlockHandle(hashBlockHandle) {}

  bool put(const stored_block_t& block) override {
    // add hash -> block record

    std::string blockHash(reinterpret_cast<const char*>(block.hash.data()),
                          block.hash.size());
    std::string blockBytes = block.toRaw();

    rocksdb::Status s = _db->Put(
        rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash, blockBytes);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    // add height -> hashes record

    std::set<hash_t> hashesList = getHashesByHeight(block.height);
    size_t hashesListSize = hashesList.size();

    std::string heightStr = std::to_string(block.height);
    hashesList.insert(block.hash);

    // nothing changed - no need for the DB update
    if (hashesList.size() == hashesListSize) {
      return true;
    }

    std::string hashesStr = hashesToString(hashesList);
    s = _db->Put(rocksdb::WriteOptions(),
                 _heightHashesHandle.get(),
                 heightStr,
                 hashesStr);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  bool getByHash(const hash_t& hash, stored_block_t* out) const override {
    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    std::string dbValue{};
    rocksdb::Status s = _db->Get(
        rocksdb::ReadOptions(), _hashBlockHandle.get(), blockHash, &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::DbError(s.ToString());
    }

    *out = stored_block_t::fromRaw(dbValue);
    return true;
  }

  bool getByHeight(height_t height,
                   std::vector<stored_block_t>* out) const override {
    bool found = false;
    std::set<hash_t> hashesList = getHashesByHeight(height);
    for (const hash_t& hash : hashesList) {
      stored_block_t outBlock{};
      if (!getByHash(hash, &outBlock)) {
        /// TODO: some information about non-existing block
        continue;
      }
      found = true;
      if (out) {
        out->push_back(outBlock);
      }
    }
    return found;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) const override {
    size_t found = 0;
    for (const hash_t& hash : hashes) {
      stored_block_t outBlock{};
      if (!getByHash(hash, &outBlock)) {
        /// TODO: some information about non-existing block
        continue;
      }
      found++;
      if (out) {
        out->push_back(outBlock);
      }
    }
    return found;
  }

  bool removeByHash(const hash_t& hash) override {
    // obtain block information
    stored_block_t outBlock{};
    if (!getByHash(hash, &outBlock)) return false;

    // obtain hashes blob for the block height
    std::set<hash_t> hashesList = getHashesByHeight(outBlock.height);

    // remove block hash from the hashes blob
    auto hashRecord = hashesList.find(hash);
    if (hashRecord != hashesList.end()) {
      hashesList.erase(hashRecord);

      std::string heightStr = std::to_string(outBlock.height);
      std::string hashesStr = hashesToString(hashesList);

      // update hashes blob in the DB
      rocksdb::Status updateStatus = _db->Put(rocksdb::WriteOptions(),
                                              _heightHashesHandle.get(),
                                              heightStr,
                                              hashesStr);
      if (!updateStatus.ok()) {
        throw db::DbError(updateStatus.ToString());
      }
    }

    return deleteBlockByHash(hash);
  }

  size_t removeByHeight(height_t height) override {
    // obtain hashes blob for the height
    std::set<hash_t> hashesList = getHashesByHeight(height);
    size_t deletedCount = 0;
    for (const hash_t& hash : hashesList) {
      if (!deleteBlockByHash(hash)) continue;
      deletedCount++;
    }

    std::string heightStr = std::to_string(height);
    rocksdb::Status s = _db->Delete(
        rocksdb::WriteOptions(), _heightHashesHandle.get(), heightStr);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    return deletedCount;
  }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return nullptr;
  }

  void commit(WriteBatch<stored_block_t>& batch) override { (void)batch; }

  std::shared_ptr<Cursor<height_t, stored_block_t>> getCursor() override {
    return nullptr;
  }

 private:
  std::shared_ptr<rocksdb::DB> _db{};
  std::shared_ptr<cf_handle_t> _heightHashesHandle{};
  std::shared_ptr<cf_handle_t> _hashBlockHandle{};

  // fetch and decode hashes blob from the DB
  std::set<hash_t> getHashesByHeight(height_t height) const {
    std::string heightStr = std::to_string(height);
    std::string dbValue{};
    rocksdb::Status s = _db->Get(
        rocksdb::ReadOptions(), _heightHashesHandle.get(), heightStr, &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return std::set<hash_t>{};
      throw db::DbError(s.ToString());
    }
    return hashesFromString(dbValue);
  }

  bool deleteBlockByHash(const hash_t& hash) {
    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    rocksdb::Status s =
        _db->Delete(rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  // decode hashes blob
  std::set<hash_t> hashesFromString(const std::string& hashesData) const {
    if (hashesData.size() % sizeof(hash_t)) {
      throw db::DbError("Not parceable hashes blob");
    }

    std::set<hash_t> result{};
    for (size_t i = 0; i < (hashesData.size() / sizeof(hash_t)); i++) {
      auto blobPart =
          hashesData.substr(i * sizeof(hash_t), (i + 1) * sizeof(hash_t));
      result.insert(blobPart);
    }
    return result;
  }

  // encode hashes into blob for DB storage
  std::string hashesToString(const std::set<hash_t>& hashes) const {
    std::string result{};
    for (hash_t hash : hashes) {
      std::string hashStr(hash.begin(), hash.end());
      result.append(hashStr);
    }
    return result;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
