// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "storage.hpp"
#ifdef WITH_ROCKSDB
#include "veriblock/pop/storage/adaptors/rocksdb_impl.hpp"
#endif
#ifdef WITH_LEVELDB
#include "veriblock/pop/storage/adaptors/leveldb_impl.hpp"
#endif
#include "validation_state.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/storage/adaptors/inmem_storage_impl.hpp"

POP_ENTITY_FREE_SIGNATURE(storage) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(storage,
                        POP_ARRAY_NAME(string) path,
                        POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(state);
  VBK_ASSERT(path.data);

  std::string str_path(path.data, path.data + path.size);
  std::shared_ptr<altintegration::adaptors::Storage> storage{nullptr};

  try {
    if (str_path == std::string(":inmem:")) {
      storage = std::make_shared<altintegration::adaptors::InmemStorageImpl>();
    } else {
#ifdef WITH_ROCKSDB
      storage =
          std::make_shared<altintegration::adaptors::RocksDBStorage>(str_path);
#endif
#ifdef WITH_LEVELDB
      storage =
          std::make_shared<altintegration::adaptors::LevelDBStorage>(str_path);
#endif
    }
  } catch (const altintegration::StorageIOException& e) {
    state->ref.Invalid("failed-create-storage", e.what());
    return nullptr;
  } catch (...) {
    VBK_ASSERT_MSG(false, "catched unexpected exception");
  }

  VBK_ASSERT_MSG(
      storage, "Storage is not initialized, you should initialize the storage");

  auto* res = new POP_ENTITY_NAME(storage);
  res->ref = std::move(storage);
  return res;
}