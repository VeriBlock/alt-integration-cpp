// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository_rocks.hpp"
#include "veriblock/storage/endorsement_repository_rocks.hpp"
#include "veriblock/storage/payloads_repository_rocks.hpp"

namespace altintegration {

enum class CF_NAMES {
  DEFAULT = 0,
  HASH_BLOCK_BTC,
  HASH_BLOCK_VBK,
  HASH_BTC_ENDORSEMENT_ID,
  HASH_VBK_ENDORSEMENT_ID,
  ALT_PAYLOADS_ID,
  VBK_PAYLOADS_ID
};

// column families in the DB
static const std::vector<std::string> cfNames{"default",
                                              "hash_block_btc",
                                              "hash_block_vbk",
                                              "hash_btc_endorsement_id",
                                              "hash_vbk_endorsement_id",
                                              "alt_payloads_id",
                                              "vbk_payloads_id"};

struct RepositoryRocksManager {
  template <typename Block_t>
  using block_repo_t = BlockRepositoryRocks<BlockIndex<Block_t>>;

  template <typename Endorsement_t>
  using endorsement_repo_t = EndorsementRepositoryRocks<Endorsement_t>;

  template <typename Payloads>
  using payloads_repo_t = PayloadsRepositoryRocks<Payloads>;

  using status_t = rocksdb::Status;

  RepositoryRocksManager(const std::string &name) : dbName(name) {}

  rocksdb::Status open() {
    // prepare column families
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    rocksdb::ColumnFamilyOptions cfOption{};
    for (const std::string &cfName : cfNames) {
      rocksdb::ColumnFamilyDescriptor descriptor(cfName, cfOption);
      column_families.push_back(descriptor);
    }

    // open DB
    rocksdb::DB *dbInstance = nullptr;
    std::vector<rocksdb::ColumnFamilyHandle *> cfHandlesData;
    rocksdb::Options options{};
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    // here is the description of this option
    // https://github.com/facebook/rocksdb/wiki/Atomic-flush
    options.atomic_flush = true;
    rocksdb::Status s = rocksdb::DB::Open(
        options, dbName, column_families, &cfHandlesData, &dbInstance);
    if (!s.ok()) return s;

    // prepare smart pointers to keep the DB state
    dbPtr = std::shared_ptr<rocksdb::DB>(dbInstance);
    cfHandles.clear();
    for (rocksdb::ColumnFamilyHandle *cfHandle : cfHandlesData) {
      auto cfHandlePtr = std::shared_ptr<rocksdb::ColumnFamilyHandle>(cfHandle);
      cfHandles.push_back(cfHandlePtr);
    }

    repoBtc = std::make_shared<block_repo_t<BtcBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<block_repo_t<VbkBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);

    repoBtcEndorsement = std::make_shared<endorsement_repo_t<BtcEndorsement>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BTC_ENDORSEMENT_ID]);

    repoVbkEndorsement = std::make_shared<endorsement_repo_t<VbkEndorsement>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_VBK_ENDORSEMENT_ID]);

    repoAltPayloads = std::make_shared<payloads_repo_t<AltPayloads>>(
        dbPtr, cfHandles[(int)CF_NAMES::ALT_PAYLOADS_ID]);

    repoVbkPayloads = std::make_shared<payloads_repo_t<VTB>>(
        dbPtr, cfHandles[(int)CF_NAMES::VBK_PAYLOADS_ID]);

    return s;
  }

  rocksdb::Status clear() {
    if (!dbPtr) {
      return rocksdb::Status::NotFound("DB is closed");
    }
    if (cfHandles.size() < cfNames.size()) {
      return rocksdb::Status::NotFound("DB is closed");
    }

    rocksdb::Status s = rocksdb::Status::OK();
    for (size_t i = (size_t)CF_NAMES::HASH_BLOCK_BTC;
         i <= (size_t)CF_NAMES::VBK_PAYLOADS_ID;
         i++) {
      auto columnName = cfHandles[i]->GetName();
      s = dbPtr->DropColumnFamily(cfHandles[i].get());
      if (!s.ok()) return s;

      rocksdb::ColumnFamilyOptions cfOption{};
      cf_handle_t *handle = nullptr;
      s = dbPtr->CreateColumnFamily(cfOption, columnName, &handle);
      if (!s.ok() && !s.IsNotFound()) {
        return s;
      }
      cfHandles[i] = std::shared_ptr<cf_handle_t>(handle);
    }

    repoBtc = std::make_shared<block_repo_t<BtcBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_BTC]);

    repoVbk = std::make_shared<block_repo_t<VbkBlock>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BLOCK_VBK]);

    repoBtcEndorsement = std::make_shared<endorsement_repo_t<BtcEndorsement>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_BTC_ENDORSEMENT_ID]);

    repoVbkEndorsement = std::make_shared<endorsement_repo_t<VbkEndorsement>>(
        dbPtr, cfHandles[(int)CF_NAMES::HASH_VBK_ENDORSEMENT_ID]);

    repoAltPayloads = std::make_shared<payloads_repo_t<AltPayloads>>(
        dbPtr, cfHandles[(int)CF_NAMES::ALT_PAYLOADS_ID]);

    repoVbkPayloads = std::make_shared<payloads_repo_t<VTB>>(
        dbPtr, cfHandles[(int)CF_NAMES::VBK_PAYLOADS_ID]);
    return s;
  }

  rocksdb::Status flush() {
    std::vector<rocksdb::ColumnFamilyHandle *> cfs(cfHandles.size());
    for (size_t i = 0; i < cfs.size(); ++i) {
      cfs[i] = cfHandles[i].get();
    }

    return dbPtr->Flush(rocksdb::FlushOptions(), cfs);
  }

  std::shared_ptr<block_repo_t<BtcBlock>> getBtcRepo() const { return repoBtc; }

  std::shared_ptr<block_repo_t<VbkBlock>> getVbkRepo() const { return repoVbk; }

  std::shared_ptr<endorsement_repo_t<BtcEndorsement>> getBtcEndorsementRepo()
      const {
    return repoBtcEndorsement;
  }

  std::shared_ptr<endorsement_repo_t<VbkEndorsement>> getVbkEndorsementRepo()
      const {
    return repoVbkEndorsement;
  }

  std::shared_ptr<payloads_repo_t<AltPayloads>> getAltPayloadsRepo() const {
    return repoAltPayloads;
  }

  std::shared_ptr<payloads_repo_t<VTB>> getVbkPayloadsRepo() const {
    return repoVbkPayloads;
  }

 private:
  std::string dbName = "";

  // smart pointers for handling DB management
  std::shared_ptr<rocksdb::DB> dbPtr;
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> cfHandles{};

  std::shared_ptr<block_repo_t<BtcBlock>> repoBtc;
  std::shared_ptr<block_repo_t<VbkBlock>> repoVbk;
  std::shared_ptr<endorsement_repo_t<BtcEndorsement>> repoBtcEndorsement;
  std::shared_ptr<endorsement_repo_t<VbkEndorsement>> repoVbkEndorsement;
  std::shared_ptr<payloads_repo_t<AltPayloads>> repoAltPayloads;
  std::shared_ptr<payloads_repo_t<VTB>> repoVbkPayloads;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_MANAGER_HPP_
