// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_

#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>

namespace altintegration {

struct StorageManager {
  virtual ~StorageManager() = default;

  virtual void flush() = 0;

  virtual void clear() = 0;

  PayloadsStorage& getPayloadsStorage() { return *_storagePayloads; }

  const PayloadsStorage& getPayloadsStorage() const {
    return *_storagePayloads;
  }

 protected:
  std::shared_ptr<PayloadsStorage> _storagePayloads;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_
