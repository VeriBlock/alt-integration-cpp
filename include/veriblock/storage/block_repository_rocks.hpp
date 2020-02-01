#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <set>

#include <rocksdb/db.h>

#include "veriblock/serde.hpp"
#include "veriblock/strutil.hpp"

#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/db_error.hpp"

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
  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                       std::shared_ptr<cf_handle_t> hashBlockHandle,
                       std::shared_ptr<cf_handle_t> heightHashesHandle)
      : _db(db),
        _hashBlockHandle(hashBlockHandle),
        _heightHashesHandle(heightHashesHandle) {}

  bool put(const stored_block_t& block) override {
    // add hash -> block record
    std::string blockHashBytes(
      reinterpret_cast<const char*>(block.hash.data()),
      block.hash.size());
    std::string blockBytes = blockToBytes(block);

    rocksdb::Status s = _db->Put(rocksdb::WriteOptions(),
                                 _hashBlockHandle.get(),
                                 blockHashBytes,
                                 blockBytes);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    // add height -> hashes record

    auto hashesList = getHashesByHeight(block.height);
    std::string heightStr = std::to_string(block.height);
    hashesList.insert(block.hash);
    std::string hashesStr = hashesToString(hashesList);

    s = _db->Put(rocksdb::WriteOptions(),
                 _heightHashesHandle.get(),
                 heightStr,
                 hashesStr);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    s = _db->Flush(rocksdb::FlushOptions());
    return s.ok();
  }

  bool getByHash(const hash_t& hash, stored_block_t* out) override {
    (void)hash;
    (void)out;
    return false;
  }

  bool getByHeight(height_t height, std::vector<stored_block_t>* out) override {
    (void)height;
    (void)out;
    return false;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) override {
    (void)hashes;
    (void)out;
    return 0;
  }

  bool removeByHash(const hash_t& hash) override {
    (void)hash;
    return false;
  }

  size_t removeByHeight(height_t height) override {
    (void)height;
    return 0;
  }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return nullptr;
  }

  void commit(WriteBatch<stored_block_t>& batch) override { (void)batch; }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::shared_ptr<cf_handle_t> _hashBlockHandle;
  std::shared_ptr<cf_handle_t> _heightHashesHandle;

  std::string blockToBytes(const stored_block_t& block) const {
    // prepare DB value from the block header bytes
    WriteStream stream;
    block.block.toRaw(stream);
    return std::string(stream.data().begin(), stream.data().end());
  }

  std::set<hash_t> getHashesByHeight(height_t height) {
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

  std::set<hash_t> hashesFromString(const std::string& hashesData) {
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

#endif  //ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
