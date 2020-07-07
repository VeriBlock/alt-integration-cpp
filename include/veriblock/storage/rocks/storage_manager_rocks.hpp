// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_STORAGE_MANAGER_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_STORAGE_MANAGER_ROCKS_HPP_

#include <veriblock/storage/rocks/payloads_storage_rocks.hpp>
#include <veriblock/storage/rocks/pop_storage_rocks.hpp>
#include <veriblock/storage/rocks/repository_rocks_manager.hpp>
#include <veriblock/storage/storage_manager.hpp>

namespace altintegration {

struct StorageManagerRocks : public StorageManager {
  StorageManagerRocks(const std::string& name) {
    _rocksManager = std::make_shared<RepositoryRocksManager>(name);
    // database schema
    _rocksManager->attachColumn("btc_blocks");
    _rocksManager->attachColumn("vbk_blocks");
    _rocksManager->attachColumn("alt_blocks");
    _rocksManager->attachColumn("tips");
    _rocksManager->attachColumn("vbk_endorsements");
    _rocksManager->attachColumn("alt_endorsements");
    _rocksManager->attachColumn("atv_payloads");
    _rocksManager->attachColumn("vtb_payloads");
    _rocksManager->attachColumn("blocks_payloads");

    auto s = _rocksManager->open();
    if (!s.ok()) throw db::StateCorruptedException(s.ToString());

    _storagePop = std::make_shared<PopStorageRocks>(*_rocksManager);
    _storagePayloads = std::make_shared<PayloadsStorageRocks>(*_rocksManager);
  }

  void flush() override {
    VBK_ASSERT(_rocksManager);
    auto s = _rocksManager->flush();
    if (!s.ok()) {
      throw db::StateCorruptedException(s.ToString());
    }
  }

  void clear() override {
    VBK_ASSERT(_rocksManager);
    auto s = _rocksManager->clear();
    if (!s.ok()) {
      throw db::StateCorruptedException(s.ToString());
    }
  }

  RepositoryRocksManager& getDbManager() { return *_rocksManager; }
  const RepositoryRocksManager& getDbManager() const { return *_rocksManager; }

 private:
  std::shared_ptr<RepositoryRocksManager> _rocksManager;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_STORAGE_MANAGER_ROCKS_HPP_
