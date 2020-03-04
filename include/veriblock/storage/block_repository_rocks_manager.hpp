#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository_rocks.hpp"

namespace VeriBlock {

enum class CF_NAMES { DEFAULT = 0, HASH_BLOCK_BTC, HASH_BLOCK_VBK };

// column families in the DB
static const std::vector<std::string> cfNames{
    "default", "hash_block_btc", "hash_block_vbk"};

struct BlockRepositoryRocksManager {
  template <typename Block_t>
  using block_repo_t = BlockRepositoryRocks<BlockIndex<Block_t>>;

  using status_t = rocksdb::Status;

  BlockRepositoryRocksManager(const std::string &name) : dbName(name) {}

  rocksdb::Status open() {
    // prepare column families
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    rocksdb::ColumnFamilyOptions cfOption{};
    for (const std::string &cfName : cfNames) {
      rocksdb::ColumnFamilyDescriptor descriptor(cfName, cfOption);
      column_families.push_back(descriptor);
    }

    // open DB
    rocksdb::DB *dbInstance = nullptr;
    std::vector<rocksdb::ColumnFamilyHandle *> cfHandlesData;
    rocksdb::Options options{};
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::Status s = rocksdb::DB::Open(
        options, dbName, column_families, &cfHandlesData, &dbInstance);
    if (!s.ok()) return s;

    // prepare smart pointers to keep the DB state
    dbPtr = std::shared_ptr<rocksdb::DB>(dbInstance);
    cfHandles.clear();
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandlesData) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
      cfHandles.push_back(cfHandlePtr);
    }

    repoBtc = std::make_shared<block_repo_t<BtcBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<block_repo_t<VbkBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);
    return s;
  }

  rocksdb::Status clear() {
    if (!dbPtr) {
      return rocksdb::Status::NotFound("DB is closed");
    }
    if (cfHandles.size() < cfNames.size()) {
      return rocksdb::Status::NotFound("DB is closed");
    }

    rocksdb::Status s = rocksdb::Status::OK();
    for (size_t i = (size_t)CF_NAMES::HASH_BLOCK_BTC;
         i <= (size_t)CF_NAMES::HASH_BLOCK_VBK;
         i++) {
      auto columnName = cfHandles[i]->GetName();
      s = dbPtr->DropColumnFamily(cfHandles[i].get());
      if (!s.ok()) return s;

      rocksdb::ColumnFamilyOptions cfOption{};
      cf_handle_t *handle = nullptr;
      s = dbPtr->CreateColumnFamily(cfOption, columnName, &handle);
      if (!s.ok() && !s.IsNotFound()) {
        return s;
      }
      cfHandles[i] = std::shared_ptr<cf_handle_t>(handle);
    }

    repoBtc = std::make_shared<block_repo_t<BtcBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<block_repo_t<VbkBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);
    return s;
  }

  std::shared_ptr<block_repo_t<BtcBlock>> getBtcRepo() const { return repoBtc; }

  std::shared_ptr<block_repo_t<VbkBlock>> getVbkRepo() const { return repoVbk; }

 private:
  std::string dbName = "";

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> cfHandles{};

  std::shared_ptr<block_repo_t<BtcBlock>> repoBtc;
  std::shared_ptr<block_repo_t<VbkBlock>> repoVbk;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
