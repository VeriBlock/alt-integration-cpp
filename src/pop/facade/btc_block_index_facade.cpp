// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/facade/btc_block_index_facade.hpp>

namespace altintegration {

const std::vector<const VbkEndorsement*>&
BtcBlockIndexFacade::getBlockOfProofEndorsements() const {
  return ptr_->blockOfProofEndorsements;
}

const std::vector<int>& BtcBlockIndexFacade::getRefs() const noexcept {
  return ptr_->getRefs();
}

BtcBlockIndexFacade::BtcBlockIndexFacade(const BtcBlockIndexFacade::impl& ptr)
    : base(ptr) {}
}  // namespace altintegration