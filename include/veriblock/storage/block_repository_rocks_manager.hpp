#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_

#include "veriblock/storage/block_repository_rocks.hpp"

namespace VeriBlock {

enum class CF_NAMES { DEFAULT = 0, HASH_BLOCK_BTC, HASH_BLOCK_VBK };

// column families in the DB
static const std::vector<std::string> cfNames{
    "default", "hash_block_btc", "hash_block_vbk"};

template <typename BlockBtc, typename BlockVbk>
struct BlockRepositoryRocksManager {
  BlockRepositoryRocksManager(const std::string &name) : dbName(name) {}

  rocksdb::Status open() {
    /// TODO: we assume DB is closed. Add a check.

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
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandlesData) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
      cfHandles.push_back(cfHandlePtr);
    }

    repoBtc = std::make_shared<BlockRepositoryRocks<BlockBtc>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<BlockRepositoryRocks<BlockVbk>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);
    return s;
  }

  rocksdb::Status clear() {
    ///TODO: we assume DB is opened. Add a check.

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

    repoBtc = std::make_shared<BlockRepositoryRocks<BlockBtc>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<BlockRepositoryRocks<BlockVbk>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);
    return s;
  }

  // block storage
  std::shared_ptr<BlockRepositoryRocks<BlockBtc>> repoBtc;
  std::shared_ptr<BlockRepositoryRocks<BlockVbk>> repoVbk;

 private:
  std::string dbName = "";

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> cfHandles{};
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
