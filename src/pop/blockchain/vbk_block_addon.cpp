// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/blockchain/vbk_block_addon.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>

namespace altintegration {

void VbkBlockAddon::setDirty() {
  static_cast<BlockIndex<VbkBlock>*>(this)->setDirty();
}

template <>
const std::vector<typename VTB::id_t>& VbkBlockAddon::getPayloadIds<VTB>()
    const {
  return _vtbids;
}

void VbkBlockAddon::setNull() {
  _refCount = 0;
  chainWork = 0;
  PopState<VbkEndorsement>::setNull();
  _vtbids.clear();
}

std::string VbkBlockAddon::toPrettyString() const {
  return fmt::format("VTBs={}[{}]", _vtbids.size(), fmt::join(_vtbids, ","));
}

void VbkBlockAddon::setIsBootstrap(bool isBootstrap) {
  if (isBootstrap) {
    // pretend this block is referenced by the genesis block of the SI chain
    addRef(0);
  } else {
    VBK_ASSERT(false && "not supported");
  }
}

void VbkBlockAddon::setRef(uint32_t count) {
  _refCount = count;
  setDirty();
}

void VbkBlockAddon::removeRef(VbkBlockAddon::ref_height_t) {
  VBK_ASSERT(_refCount > 0 &&
             "state corruption: attempted to remove a nonexitent reference "
             "to a VBK block");
  --_refCount;
  setDirty();
}

void VbkBlockAddon::addRef(VbkBlockAddon::ref_height_t) {
  ++_refCount;
  setDirty();
}

void VbkBlockAddon::setNullInmemFields() {
  chainWork = 0;
  blockOfProofEndorsements.clear();
  endorsedBy.clear();
}

}  // namespace altintegration
