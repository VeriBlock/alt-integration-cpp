// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_ROCKS_HPP_

#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/block_repository_rocks.hpp>
#include <veriblock/storage/payloads_repository_rocks.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>
#include <veriblock/storage/tips_repository_rocks.hpp>
#include <veriblock/storage/storage_manager.hpp>

namespace altintegration {

struct StorageManagerRocks  : public StorageManager {
  StorageManagerRocks(const std::string &name = "") {
    rocksManager = std::make_shared<RepositoryRocksManager>(name);
    // database schema
    rocksManager->attachColumn("btc_blocks");
    rocksManager->attachColumn("vbk_blocks");
    rocksManager->attachColumn("alt_blocks");
    rocksManager->attachColumn("tips");
    rocksManager->attachColumn("vbk_endorsements");
    rocksManager->attachColumn("alt_endorsements");
    rocksManager->attachColumn("atv_payloads");
    rocksManager->attachColumn("vtb_payloads");
    rocksManager->attachColumn("blocks_payloads");

    auto s = rocksManager->open();
    if (!s.ok()) throw db::DbError(s.ToString());
  }

  void flush() override {
    if (rocksManager == nullptr) return;
    auto s = rocksManager->flush();
    if (!s.ok()) throw db::DbError(s.ToString());
  }

  PayloadsStorage newPayloadsStorage() override {
    auto repoAtv =
        std::make_shared<PayloadsRepositoryRocks<ATV>>(*rocksManager, "atv_payloads");
    auto repoVtb =
        std::make_shared<PayloadsRepositoryRocks<VTB>>(*rocksManager, "vtb_payloads");
    auto repoBlocks =
        std::make_shared<PayloadsRepositoryRocks<VbkBlock>>(*rocksManager, "blocks_payloads");
    return PayloadsStorage(repoAtv, repoVtb, repoBlocks);
  }

  PopStorage newPopStorage() override {
    auto repoBtc = std::make_shared<BlockRepositoryRocks<BlockIndex<BtcBlock>>>(
        *rocksManager, "btc_blocks");
    auto repoVbk = std::make_shared<BlockRepositoryRocks<BlockIndex<VbkBlock>>>(
        *rocksManager, "vbk_blocks");
    auto repoAlt = std::make_shared<BlockRepositoryRocks<BlockIndex<AltBlock>>>(
        *rocksManager, "alt_blocks");
    auto repoTipsBtc =
        std::make_shared<TipsRepositoryRocks<BlockIndex<BtcBlock>>>(
            *rocksManager, "tips");
    auto repoTipsVbk =
        std::make_shared<TipsRepositoryRocks<BlockIndex<VbkBlock>>>(
            *rocksManager, "tips");
    auto repoTipsAlt =
        std::make_shared<TipsRepositoryRocks<BlockIndex<AltBlock>>>(
            *rocksManager, "tips");
    auto erepoVbk = std::make_shared<PayloadsRepositoryRocks<VbkEndorsement>>(
        *rocksManager, "vbk_endorsements");
    auto erepoAlt = std::make_shared<PayloadsRepositoryRocks<AltEndorsement>>(
        *rocksManager, "alt_endorsements");

    return PopStorage(repoBtc,
                      repoVbk,
                      repoAlt,
                      repoTipsBtc,
                      repoTipsVbk,
                      repoTipsAlt,
                      erepoVbk,
                      erepoAlt);
  }

 private:
  std::shared_ptr<RepositoryRocksManager> rocksManager;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_ROCKS_HPP_
