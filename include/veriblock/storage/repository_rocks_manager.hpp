// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_

namespace altintegration {

struct RepositoryRocksManager {
  using status_t = rocksdb::Status;

  RepositoryRocksManager(const std::string &name) : dbName(name) {}

  void attachColumn(const std::string& name) {
    rocksdb::ColumnFamilyOptions options{};
    rocksdb::ColumnFamilyDescriptor descriptor(name, options);
    columnFamilies.push_back(descriptor);
  }

  rocksdb::ColumnFamilyHandle* getColumn(const std::string &name) {
    auto it =
        std::find_if(columnHandles.begin(),
                     columnHandles.end(),
                     [&name](std::shared_ptr<rocksdb::ColumnFamilyHandle> p) {
                       return p->GetName() == name;
                     });
    if (it == columnHandles.end())
      throw db::DbError(std::string("Column handle with name ") + name +
                        std::string(" not found"));
    return it->get();
  }

  rocksdb::DB* getDB() { return dbPtr.get(); }

  rocksdb::Status open() {
    rocksdb::DB *dbInstance = nullptr;
    std::vector<rocksdb::ColumnFamilyHandle *> cfHandlesData;
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
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandlesData) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
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
    std::vector<rocksdb::ColumnFamilyHandle *> cfs(columnHandles.size());
    for (size_t i = 0; i < cfs.size(); ++i) {
      cfs[i] = columnHandles[i].get();
    }

    return dbPtr->Flush(rocksdb::FlushOptions(), cfs);
  }

 private:
  std::string dbName = "";

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> columnHandles{};
  std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies{};
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
