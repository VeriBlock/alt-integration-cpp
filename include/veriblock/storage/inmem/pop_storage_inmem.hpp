// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_POP_STORAGE_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_POP_STORAGE_INMEM_HPP_

#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/inmem/block_repository_inmem.hpp>
#include <veriblock/storage/inmem/payloads_repository_inmem.hpp>
#include <veriblock/storage/inmem/tips_repository_inmem.hpp>

namespace altintegration {

class PopStorageInmem : public PopStorage {
 public:
  virtual ~PopStorageInmem() = default;
  PopStorageInmem() {
    _brepoBtc = std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    _brepoVbk = std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    _brepoAlt = std::make_shared<BlockRepositoryInmem<BlockIndex<AltBlock>>>();
    _trepoBtc = std::make_shared<TipsRepositoryInmem<BlockIndex<BtcBlock>>>();
    _trepoVbk = std::make_shared<TipsRepositoryInmem<BlockIndex<VbkBlock>>>();
    _trepoAlt = std::make_shared<TipsRepositoryInmem<BlockIndex<AltBlock>>>();
    _erepoVbk = std::make_shared<PayloadsRepositoryInmem<VbkEndorsement>>();
    _erepoAlt = std::make_shared<PayloadsRepositoryInmem<AltEndorsement>>();
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_POP_STORAGE_INMEM_HPP_