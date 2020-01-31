#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include "veriblock/serde.hpp"
#include "veriblock/strutil.hpp"

#include "veriblock/storage/block_repository.hpp"

namespace VeriBlock {

template <typename Block>
class BlockRepositoryRocks : public BlockRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

 public:
  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db) : _db(db) {}

  bool put(const stored_block_t& block) override {
    // prepare DB key from the block hash bytes
    std::vector<uint8_t> blockHash = block.hash.asVector();
    char* blockHashBytes = reinterpret_cast<char*>(blockHash.data());
    rocksdb::Slice dbKey(blockHashBytes, blockHash.size());

    // prepare DB value from the block header bytes
    WriteStream stream;
    block.block.toRaw(stream);
    // stream data is read only so make a copy first
    std::vector<uint8_t> tempStreamData(stream.data().begin(), stream.data().end());
    char* blockBytes = reinterpret_cast<char*>(tempStreamData.data());
    rocksdb::Slice dbValue(blockBytes, tempStreamData.size());
    rocksdb::Status s = _db->Put(rocksdb::WriteOptions(), dbKey, dbValue);
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
};

}  // namespace VeriBlock

#endif  //ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
