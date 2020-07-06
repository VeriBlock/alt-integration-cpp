// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_POP_STORAGE_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_POP_STORAGE_ROCKS_HPP_

#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/rocks/tips_repository_rocks.hpp>
#include <veriblock/storage/rocks/block_repository_rocks.hpp>
#include <veriblock/storage/rocks/payloads_repository_rocks.hpp>

namespace altintegration {

class PopStorageRocks : public PopStorage {
 public:
  virtual ~PopStorageRocks() = default;
  PopStorageRocks(RepositoryRocksManager& manager) {
    _brepoBtc = std::make_shared<BlockRepositoryRocks<BlockIndex<BtcBlock>>>(
        manager, "btc_blocks");
    _brepoVbk = std::make_shared<BlockRepositoryRocks<BlockIndex<VbkBlock>>>(
        manager, "vbk_blocks");
    _brepoAlt = std::make_shared<BlockRepositoryRocks<BlockIndex<AltBlock>>>(
        manager, "alt_blocks");
    _trepoBtc = std::make_shared<TipsRepositoryRocks<BlockIndex<BtcBlock>>>(
        manager, "tips");
    _trepoVbk = std::make_shared<TipsRepositoryRocks<BlockIndex<VbkBlock>>>(
        manager, "tips");
    _trepoAlt = std::make_shared<TipsRepositoryRocks<BlockIndex<AltBlock>>>(
        manager, "tips");
    _erepoVbk = std::make_shared<PayloadsRepositoryRocks<VbkEndorsement>>(
        manager, "vbk_endorsements");
    _erepoAlt = std::make_shared<PayloadsRepositoryRocks<AltEndorsement>>(
        manager, "alt_endorsements");
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_POP_STORAGE_ROCKS_HPP_