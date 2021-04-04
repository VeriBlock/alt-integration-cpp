// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/storage/stored_vbk_block_addon.hpp>

namespace altintegration {

StoredVbkBlockAddon::StoredVbkBlockAddon(const addon_t& other) {
  endorsedByHashes = map_get_id_from_pointers<uint256, const VbkEndorsement>(
      other.getEndorsedBy());
  blockOfProofEndorsementHashes =
      map_get_id_from_pointers<uint256, const AltEndorsement>(
          other.getBlockOfProofEndorsement());
  _refCount = other.refCount();
  _vtbids = other.getPayloadIds<VTB>();
  popState = other;
}

void StoredVbkBlockAddon::toVbkEncoding(WriteStream& w) const {
  writeArrayOf<uint256>(w, endorsedByHashes, writeSingleByteLenValue);
  writeArrayOf<uint256>(
      w, blockOfProofEndorsementHashes, writeSingleByteLenValue);
  w.writeBE<uint32_t>(_refCount);
  writeArrayOf<VTB::id_t>(w, _vtbids, [](WriteStream& w, const VTB::id_t& u) {
    writeSingleByteLenValue(w, u);
  });
  popState.toVbkEncoding(w);
}

void StoredVbkBlockAddon::toInmem(StoredVbkBlockAddon::addon_t& to) const {
  to.setRef(_refCount);
  to.template insertPayloadIds<VTB>(_vtbids);
  auto& p = static_cast<pop_state_t&>(to);
  p = popState;
}

std::string StoredVbkBlockAddon::toPrettyString() const {
  return fmt::format("VTBs={}[{}]", _vtbids.size(), fmt::join(_vtbids, ","));
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredVbkBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<uint256>(
          stream,
          out.endorsedByHashes,
          state,
          0,
          MAX_POPDATA_VBK,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("stored-vbk-block-addon-bad-endorsedby-hash");
  }

  if (!readArrayOf<uint256>(
          stream,
          out.blockOfProofEndorsementHashes,
          state,
          0,
          MAX_POPDATA_VBK,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("stored-vbk-block-addon-bad-block-of-proof-hash");
  }

  if (!stream.readBE<uint32_t>(out._refCount, state)) {
    return state.Invalid("stored-vbk-addon-bad-ref-count");
  }

  if (!readArrayOf<VTB::id_t>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_VBKPOPTX_PER_VBK_BLOCK,
          [](ReadStream& stream, VTB::id_t& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, VTB::id_t::size(), VTB::id_t::size());
          })) {
    return state.Invalid("stored-vbk-addon-bad-vtbid");
  }

  if (!DeserializeFromVbkEncoding(stream, out.popState, state)) {
    return state.Invalid("stored-vbk-block-addon-bad-popstate");
  }

  return true;
}

}  // namespace altintegration
