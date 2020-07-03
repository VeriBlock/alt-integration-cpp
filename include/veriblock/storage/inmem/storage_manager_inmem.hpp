// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_STORAGE_MANAGER_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_STORAGE_MANAGER_INMEM_HPP_

#include <veriblock/storage/inmem/pop_storage_inmem.hpp>
#include <veriblock/storage/inmem/payloads_storage_inmem.hpp>
#include <veriblock/storage/storage_manager.hpp>

namespace altintegration {

struct StorageManagerInmem : public StorageManager {
  StorageManagerInmem() {
    _storagePop = std::make_shared<PopStorageInmem>();
    _storagePayloads = std::make_shared<PayloadsStorageInmem>();
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_STORAGE_MANAGER_INMEM_HPP_
