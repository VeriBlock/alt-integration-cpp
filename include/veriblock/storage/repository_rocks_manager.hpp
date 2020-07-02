// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_REPOSITORY_ROCKS_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_REPOSITORY_ROCKS_MANAGER_HPP_

#include <rocksdb/db.h>
#include <veriblock/storage/db_error.hpp>

namespace altintegration {

struct RepositoryRocksManager {
  using status_t = rocksdb::Status;
  //! column family type
  using cf_handle_t = rocksdb::ColumnFamilyHandle;

  RepositoryRocksManager(const std::string &name) : dbName(name) {}

  void attachColumn(const std::string& name) {
    rocksdb::ColumnFamilyOptions options{};
    rocksdb::ColumnFamilyDescriptor descriptor(name, options);
    columnFamilies.push_back(descriptor);
  }

  cf_handle_t *getColumn(const std::string &name) {
    auto it =
        std::find_if(columnHandles.begin(),
                     columnHandles.end(),
                           [&name](std::shared_ptr<cf_handle_t> p) {
                       return p->GetName() == name;
                     });
    if (it == columnHandles.end()) return nullptr;
    return it->get();
  }

  rocksdb::DB* getDB() { return dbPtr.get(); }

  rocksdb::Status open() {
    rocksdb::DB *dbInstance = nullptr;
    std::vector<cf_handle_t *> cfHandlesData;
    rocksdb::Options options{};
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    // here is the description of this option
    // https://github.com/facebook/rocksdb/wiki/Atomic-flush
    options.atomic_flush = true;
    attachColumn("default");
    rocksdb::Status s = rocksdb::DB::Open(
        options, dbName, columnFamilies, &cfHandlesData, &dbInstance);
    if (!s.ok()) return s;

    // prepare smart pointers to keep the DB state
    dbPtr = std::shared_ptr<rocksdb::DB>(dbInstance);
    columnHandles.clear();
    for (cf_handle_t *cfHandle : cfHandlesData) {
      auto cfHandlePtr = std::shared_ptr<cf_handle_t>(cfHandle);
      columnHandles.push_back(cfHandlePtr);
    }
    return s;
  }

  rocksdb::Status clear() {
    if (!dbPtr) {
      return rocksdb::Status::NotFound("DB is closed");
    }

    rocksdb::Status s = rocksdb::Status::OK();
    for (size_t i = 0; i < columnHandles.size(); ++i) {
      auto columnName = columnHandles[i]->GetName();
      if (columnName == "default") continue;
      s = dbPtr->DropColumnFamily(columnHandles[i].get());
      if (!s.ok()) return s;

      rocksdb::ColumnFamilyOptions cfOption{};
      cf_handle_t *handle = nullptr;
      s = dbPtr->CreateColumnFamily(cfOption, columnName, &handle);
      if (!s.ok() && !s.IsNotFound()) {
        return s;
      }
      columnHandles[i] = std::shared_ptr<cf_handle_t>(handle);
    }
    return s;
  }

  rocksdb::Status flush() {
    std::vector<cf_handle_t *> cfs(columnHandles.size());
    for (size_t i = 0; i < cfs.size(); ++i) {
      cfs[i] = columnHandles[i].get();
    }

    return dbPtr->Flush(rocksdb::FlushOptions(), cfs);
  }

 private:
  std::string dbName = "";

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<cf_handle_t>> columnHandles{};
  std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies{};
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_REPOSITORY_ROCKS_MANAGER_HPP_
