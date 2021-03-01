// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/facade/vbk_block_index_facade.hpp>

namespace altintegration {

const std::vector<const AltEndorsement*>&
VbkBlockIndexFacade::getBlockOfProofEndorsements() const {
  return ptr_->blockOfProofEndorsements;
}

VbkBlockIndexFacade::VbkBlockIndexFacade(const VbkBlockIndexFacade::impl& ptr)
    : base(ptr) {}

const std::vector<const VbkEndorsement*>
VbkBlockIndexFacade::getContainingEndorsements() const {
  auto& s = ptr_->getContainingEndorsements();

  std::vector<const VbkEndorsement*> ret;
  ret.reserve(s.size());

  for (auto& p : s) {
    ret.push_back(p.second.get());
  }
  return ret;
}

const std::vector<const VbkEndorsement*> VbkBlockIndexFacade::getEndorsedBy()
    const {
  return ptr_->getEndorsedBy();
}

uint32_t VbkBlockIndexFacade::getRefCount() const { return ptr_->refCount(); }
}  // namespace altintegration