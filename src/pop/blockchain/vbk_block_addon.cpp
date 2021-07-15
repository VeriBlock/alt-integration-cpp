// Copyright (c) 2019-2021 Xenios SEZC
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

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkBlockAddon& out,
                                ValidationState& state) {
  if (!stream.readBE<uint32_t>(out._refCount, state)) {
    return state.Invalid("vbk-addon-bad-ref-count");
  }

  PopState<VbkEndorsement>& pop = out;
  if (!DeserializeFromVbkEncoding(stream, pop, state)) {
    return state.Invalid("vbk-addon-bad-popstate");
  }

  if (!readArrayOf<uint256>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_VBKPOPTX_PER_VBK_BLOCK,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("vbk-addon-bad-vtbid");
  }

  return true;
}

void VbkBlockAddon::toVbkEncoding(WriteStream& w) const {
  w.writeBE<uint32_t>(_refCount);
  const PopState<VbkEndorsement>* e = this;
  e->toVbkEncoding(w);
  writeArrayOf<uint256>(w, _vtbids, [](WriteStream& w, const uint256& u) {
    writeSingleByteLenValue(w, u);
  });
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
    // fix MSVC 4127 warning
    bool valid = false;
    VBK_ASSERT(valid && "not supported");
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

void VbkBlockAddon::insertBlockOfProofEndorsement(const AltEndorsement* e) {
  this->_blockOfProofEndorsements.push_back(e);
  setDirty();
}

bool VbkBlockAddon::eraseLastFromBlockOfProofEndorsement(
    const AltEndorsement* endorsement) {
  auto rm = [&endorsement](const AltEndorsement* e) -> bool {
    return e == endorsement;
  };
  auto res =
      erase_last_item_if<const AltEndorsement*>(_blockOfProofEndorsements, rm);
  if (res) {
    setDirty();
  }
  return res;
}

void VbkBlockAddon::clearBlockOfProofEndorsement() {
  this->_blockOfProofEndorsements.clear();
  setDirty();
}

const std::vector<const AltEndorsement*>&
VbkBlockAddon::getBlockOfProofEndorsement() const {
  return this->_blockOfProofEndorsements;
}

void VbkBlockAddon::setNullInmemFields() {
  chainWork = 0;
  _blockOfProofEndorsements.clear();
  _endorsedBy.clear();
}

}  // namespace altintegration
