// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_

#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/payloads_repository_inmem.hpp>
#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/tips_repository_inmem.hpp>

#ifdef VERIBLOCK_WITH_ROCKSDB
#include <veriblock/storage/block_repository_rocks.hpp>
#include <veriblock/storage/payloads_repository_rocks.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>
#include <veriblock/storage/tips_repository_rocks.hpp>
#endif  // VERIBLOCK_WITH_ROCKSDB

namespace altintegration {

struct StorageManager {
#ifdef VERIBLOCK_WITH_ROCKSDB
  StorageManager(const std::string &name = "alt-cpp-db")
      : rocksManager(std::make_shared<RepositoryRocksManager>(name)) {}
#else   //! VERIBLOCK_WITH_ROCKSDB
  StorageManager(const std::string &/*name*/) {}
#endif  // VERIBLOCK_WITH_ROCKSDB

#ifdef VERIBLOCK_WITH_ROCKSDB
  void openRocks() {
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

  void flushRocks() {
    auto s = rocksManager->flush();
    if (!s.ok()) throw db::DbError(s.ToString());
  }

  void clearRocks() {
    auto s = rocksManager->clear();
    if (!s.ok()) throw db::DbError(s.ToString());
  }

  PayloadsStorage newPayloadsStorageRocks() {
    auto *db = rocksManager->getDB();
    auto *column = rocksManager->getColumn("atv_payloads");
    auto repoAtv = std::make_shared<PayloadsRepositoryRocks<ATV>>(db, column);
    column = rocksManager->getColumn("vtb_payloads");
    auto repoVtb = std::make_shared<PayloadsRepositoryRocks<VTB>>(db, column);
    column = rocksManager->getColumn("blocks_payloads");
    auto repoBlocks =
        std::make_shared<PayloadsRepositoryRocks<VbkBlock>>(db, column);
    return PayloadsStorage(repoAtv, repoVtb, repoBlocks);
  }

  PopStorage newPopStorageRocks() {
    auto *db = rocksManager->getDB();
    auto *column = rocksManager->getColumn("btc_blocks");
    auto repoBtc = std::make_shared<BlockRepositoryRocks<BlockIndex<BtcBlock>>>(
        db, column);
    column = rocksManager->getColumn("vbk_blocks");
    auto repoVbk = std::make_shared<BlockRepositoryRocks<BlockIndex<VbkBlock>>>(
        db, column);
    column = rocksManager->getColumn("alt_blocks");
    auto repoAlt = std::make_shared<BlockRepositoryRocks<BlockIndex<AltBlock>>>(
        db, column);
    column = rocksManager->getColumn("tips");
    auto repoTipsBtc =
        std::make_shared<TipsRepositoryRocks<BlockIndex<BtcBlock>>>(db, column);
    auto repoTipsVbk =
        std::make_shared<TipsRepositoryRocks<BlockIndex<VbkBlock>>>(db, column);
    auto repoTipsAlt =
        std::make_shared<TipsRepositoryRocks<BlockIndex<AltBlock>>>(db, column);
    column = rocksManager->getColumn("vbk_endorsements");
    auto erepoVbk =
        std::make_shared<PayloadsRepositoryRocks<VbkEndorsement>>(db, column);
    column = rocksManager->getColumn("alt_endorsements");
    auto erepoAlt =
        std::make_shared<PayloadsRepositoryRocks<AltEndorsement>>(db, column);

    return PopStorage(repoBtc,
                      repoVbk,
                      repoAlt,
                      repoTipsBtc,
                      repoTipsVbk,
                      repoTipsAlt,
                      erepoVbk,
                      erepoAlt);
  }
#endif  // VERIBLOCK_WITH_ROCKSDB

  PayloadsStorage newPayloadsStorageInmem() {
    auto repoAtv = std::make_shared<PayloadsRepositoryInmem<ATV>>();
    auto repoVtb = std::make_shared<PayloadsRepositoryInmem<VTB>>();
    auto repoBlocks = std::make_shared<PayloadsRepositoryInmem<VbkBlock>>();
    return PayloadsStorage(repoAtv, repoVtb, repoBlocks);
  }

  PopStorage newPopStorageInmem() {
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

 private:
#ifdef VERIBLOCK_WITH_ROCKSDB
  std::shared_ptr<RepositoryRocksManager> rocksManager;
#endif  // VERIBLOCK_WITH_ROCKSDB
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORAGE_MANAGER_HPP_
