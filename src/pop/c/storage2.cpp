// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "storage2.hpp"
#ifdef WITH_ROCKSDB
#include "adaptors/rocksdb_impl.hpp"
#endif
#ifdef WITH_LEVELDB
#include "adaptors/leveldb_impl.hpp"
#endif
#include "adaptors/inmem_storage_impl.hpp"
#include "validation_state2.hpp"
#include "veriblock/pop/assert.hpp"

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

  auto* res = new POP_ENTITY_NAME(storage);
  std::string str_path(path.data, path.data + path.size);

  try {
    if (str_path == std::string(":inmem:")) {
      res->ref = std::make_shared<adaptors::InmemStorageImpl>();
    } else {
#ifdef WITH_ROCKSDB
      res->ref = std::make_shared<adaptors::RocksDBStorage>(str_path);
#endif
#ifdef WITH_LEVELDB
      res->ref = std::make_shared<adaptors::LevelDBStorage>(str_path);
#endif
    }
  } catch (const altintegration::StorageIOException& e) {
    state->ref.Invalid("failed-create-storage", e.what());
    return nullptr;
  } catch (...) {
    VBK_ASSERT_MSG(false, "catched unexpected exception");
  }

  VBK_ASSERT_MSG(
      res->ref,
      "Storage is not initialized, you should initialize the storage");

  return res;
}