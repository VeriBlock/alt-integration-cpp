// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/facade/alt_block_index_facade.hpp>

namespace altintegration {

AltBlockIndexFacade::AltBlockIndexFacade(const AltBlockIndexFacade::impl& ptr)
    : base(ptr) {}

std::vector<const AltEndorsement*>
AltBlockIndexFacade::getContainingEndorsements() const {
  auto& s = ptr_->getContainingEndorsements();

  std::vector<const AltEndorsement*> ret;
  ret.reserve(s.size());

  for (auto& p : s) {
    ret.push_back(p.second.get());
  }
  return ret;
}

std::vector<const AltEndorsement*> AltBlockIndexFacade::getEndorsedBy()
    const noexcept {
  return ptr_->getEndorsedBy();
}

const std::vector<ATV::id_t>& AltBlockIndexFacade::getContainingATVs()
    const noexcept {
  return ptr_->getPayloadIds<ATV>();
}

const std::vector<VTB::id_t>& AltBlockIndexFacade::getContainingVTBs()
    const noexcept {
  return ptr_->getPayloadIds<VTB>();
}

const std::vector<VbkBlock::id_t>& AltBlockIndexFacade::getContainingVbkBlocks()
    const noexcept {
  return ptr_->getPayloadIds<VbkBlock>();
}

}  // namespace altintegration