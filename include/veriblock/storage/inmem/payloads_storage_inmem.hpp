// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_

#include <veriblock/storage/inmem/repository_inmem.hpp>
#include <veriblock/storage/payloads_storage.hpp>

namespace altintegration {

class PayloadsStorageInmem : public PayloadsStorage {
 public:
  virtual ~PayloadsStorageInmem() = default;

  PayloadsStorageInmem()
      : PayloadsStorage(std::make_shared<RepositoryInmem>()) {}
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_