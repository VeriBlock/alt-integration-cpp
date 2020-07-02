// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_INMEM_HPP_

#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/payloads_repository_inmem.hpp>
#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/tips_repository_inmem.hpp>
#include <veriblock/storage/storage_manager.hpp>

namespace altintegration {

struct StorageManagerInmem : public StorageManager {
  StorageManagerInmem() {}

  void flush() override {}

  PayloadsStorage newPayloadsStorage() override {
    auto repoAtv = std::make_shared<PayloadsRepositoryInmem<ATV>>();
    auto repoVtb = std::make_shared<PayloadsRepositoryInmem<VTB>>();
    auto repoBlocks = std::make_shared<PayloadsRepositoryInmem<VbkBlock>>();
    return PayloadsStorage(repoAtv, repoVtb, repoBlocks);
  }

  PopStorage newPopStorage() override {
    auto repoBtc =
        std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    auto repoVbk =
        std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    auto repoAlt =
        std::make_shared<BlockRepositoryInmem<BlockIndex<AltBlock>>>();
    auto repoTipsBtc =
        std::make_shared<TipsRepositoryInmem<BlockIndex<BtcBlock>>>();
    auto repoTipsVbk =
        std::make_shared<TipsRepositoryInmem<BlockIndex<VbkBlock>>>();
    auto repoTipsAlt =
        std::make_shared<TipsRepositoryInmem<BlockIndex<AltBlock>>>();
    auto erepoVbk = std::make_shared<PayloadsRepositoryInmem<VbkEndorsement>>();
    auto erepoAlt = std::make_shared<PayloadsRepositoryInmem<AltEndorsement>>();
    return PopStorage(repoBtc,
                      repoVbk,
                      repoAlt,
                      repoTipsBtc,
                      repoTipsVbk,
                      repoTipsAlt,
                      erepoVbk,
                      erepoAlt);
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_INMEM_HPP_
