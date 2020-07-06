// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_

#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/inmem/payloads_repository_inmem.hpp>

namespace altintegration {

class PayloadsStorageInmem : public PayloadsStorage {
 public:
  virtual ~PayloadsStorageInmem() = default;

  PayloadsStorageInmem() {
    _repoAtv = std::make_shared<PayloadsRepositoryInmem<ATV>>();
    _repoVtb = std::make_shared<PayloadsRepositoryInmem<VTB>>();
    _repoBlocks = std::make_shared<PayloadsRepositoryInmem<VbkBlock>>();
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_PAYLOADS_STORAGE_INMEM_HPP_