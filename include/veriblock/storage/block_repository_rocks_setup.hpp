#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_SETUP_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_SETUP_HPP_

#include "veriblock/storage/block_repository_rocks.hpp"

namespace VeriBlock {

enum class CF_NAMES { DEFAULT = 0, HASH_BLOCK_BTC, HASH_BLOCK_VBK };

// column families in the DB
static const std::vector<std::string> cfNames{
    "default", "hash_block_btc", "hash_block_vbk"};

template <typename BlockBtc, typename BlockVbk>
struct BlockRepositoryRocksInstance {
  BlockRepositoryRocksInstance(const std::string &name) : dbName(name) {}

  rocksdb::Status Open() {
    rocksdb::Status s = openDB();
    if (!s.ok()) return s;

    // prepare smart pointers to look after DB cleanup
    dbPtr = std::shared_ptr<rocksdb::DB>(dbInstance);
    cfHandlePtrs = std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>>();
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandles) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
      cfHandlePtrs.push_back(cfHandlePtr);
    }

    repoBtc = std::make_shared<BlockRepositoryRocks<BlockBtc>>(
        dbPtr, cfHandlePtrs[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<BlockRepositoryRocks<BlockVbk>>(
        dbPtr, cfHandlePtrs[(int)CF_NAMES::HASH_BLOCK_VBK]);
    return s;
  }

  // block storage
  std::shared_ptr<BlockRepositoryRocks<BlockBtc>> repoBtc;
  std::shared_ptr<BlockRepositoryRocks<BlockVbk>> repoVbk;

 private:
  std::string dbName = "";
  rocksdb::DB *dbInstance = nullptr;
  std::vector<rocksdb::ColumnFamilyHandle *> cfHandles{};

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> cfHandlePtrs;

  rocksdb::Status openDB() {
    // prepare column families
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    rocksdb::ColumnFamilyOptions cfOption{};
    for (const std::string &cfName : cfNames) {
      rocksdb::ColumnFamilyDescriptor descriptor(cfName, cfOption);
      column_families.push_back(descriptor);
    }
    cfHandles.clear();
    rocksdb::Options options{};
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::Status s = rocksdb::DB::Open(
        options, dbName, column_families, &cfHandles, &dbInstance);
    if (!s.ok()) return s;
    return s;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_SETUP_HPP_
