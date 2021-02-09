// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "storage.hpp"
#include "veriblock/assert.hpp"
#include "veriblock/c/storage.h"
#ifdef WITH_ROCKSDB
#include "adaptors/rocksdb_impl.hpp"
#endif
#include "adaptors/inmem_storage_impl.hpp"
#include "validation_state.hpp"
#include "veriblock/exceptions/storage_io.hpp"

Storage_t* VBK_NewStorage(const char* path, VbkValidationState* state) {
  VBK_ASSERT(path);
  VBK_ASSERT(state);

  auto* v = new Storage();

  try {
    if (std::string(path) == std::string(":inmem:")) {
      v->storage = std::make_shared<adaptors::InmemStorageImpl>();
    } else {
#ifdef WITH_ROCKSDB
      v->storage = std::make_shared<adaptors::RocksDBStorage>(path);
#endif
    }
  } catch (const altintegration::StorageIOException& e) {
    state->getState().Invalid("failed-create-storage", e.what());
    return nullptr;
  } catch (...) {
    VBK_ASSERT_MSG(false, "catched unexpected exception");
  }

  VBK_ASSERT_MSG(
      v->storage,
      "Storage is not initialized, you should initialize the storage");

  return v;
}

void VBK_FreeStorage(Storage_t* storage) {
  if (storage != nullptr) {
    delete storage;
    storage = nullptr;
  }
}
