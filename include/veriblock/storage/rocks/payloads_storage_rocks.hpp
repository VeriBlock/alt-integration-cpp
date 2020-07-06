// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_STORAGE_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_STORAGE_ROCKS_HPP_

#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/rocks/payloads_repository_rocks.hpp>

namespace altintegration {

class PayloadsStorageRocks : public PayloadsStorage {
 public:
  virtual ~PayloadsStorageRocks() = default;

  PayloadsStorageRocks(RepositoryRocksManager& manager) {
    _repoAtv =
        std::make_shared<PayloadsRepositoryRocks<ATV>>(manager, "atv_payloads");
    _repoVtb =
        std::make_shared<PayloadsRepositoryRocks<VTB>>(manager, "vtb_payloads");
    _repoBlocks = std::make_shared<PayloadsRepositoryRocks<VbkBlock>>(
        manager, "blocks_payloads");
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_PAYLOADS_STORAGE_ROCKS_HPP_