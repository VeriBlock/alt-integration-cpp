#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <set>

#include "veriblock/blob.hpp"
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
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

  //! column family type
  using cf_handle_t = rocksdb::ColumnFamilyHandle;

 public:
  BlockRepositoryRocks() = default;

  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                       std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(db),
        _hashBlockHandle(hashBlockHandle) {}

  bool put(const stored_block_t& block) override {
    std::string blockHash(reinterpret_cast<const char*>(block.hash.data()),
                          block.hash.size());
    std::string blockBytes = block.toRaw();

    rocksdb::Status s = _db->Put(
        rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash, blockBytes);
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
    stored_block_t outBlock{};
    bool existing = getByHash(hash, &outBlock);
    if (!existing) return false;

    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    rocksdb::Status s =
        _db->Delete(rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  void clear() override { }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return nullptr;
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return nullptr;
  }

 private:
  std::shared_ptr<rocksdb::DB> _db{};
  std::shared_ptr<cf_handle_t> _hashBlockHandle{};
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
