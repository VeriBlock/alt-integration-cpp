// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/storage/stored_vbk_block_addon.hpp>

namespace altintegration {

StoredVbkBlockAddon::StoredVbkBlockAddon(const addon_t& other) {
  endorsedByIds =
      map_get_id_from_pointers<VbkEndorsement::id_t, const VbkEndorsement>(
          other.getEndorsedBy());
  blockOfProofEndorsementIds =
      map_get_id_from_pointers<AltEndorsement::id_t, const AltEndorsement>(
          other.getBlockOfProofEndorsement());
  _refCount = other.refCount();
  _vtbids = other.getPayloadIds<VTB>();
  popState = other;
}

void StoredVbkBlockAddon::toVbkEncoding(WriteStream& w) const {
  writeContainer(w, endorsedByIds, writeSingleByteLenValue);
  writeContainer(w, blockOfProofEndorsementIds, writeSingleByteLenValue);
  w.writeBE<uint32_t>(_refCount);
  writeContainer(w, _vtbids, [](WriteStream& w, const VTB::id_t& u) {
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
  return altintegration::format(
      "VTBs={}[{}]", _vtbids.size(), fmt::join(_vtbids, ","));
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredVbkBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<VbkEndorsement::id_t>(stream,
                                         out.endorsedByIds,
                                         state,
                                         0,
                                         MAX_POPDATA_VBK,
                                         [](ReadStream& stream,
                                            VbkEndorsement::id_t& o,
                                            ValidationState& state) -> bool {
                                           return readSingleByteLenValue(
                                               stream,
                                               o,
                                               state,
                                               VbkEndorsement::id_t::size(),
                                               VbkEndorsement::id_t::size());
                                         })) {
    return state.Invalid("stored-vbk-block-addon-bad-endorsedby-hash");
  }

  if (!readArrayOf<AltEndorsement::id_t>(stream,
                                         out.blockOfProofEndorsementIds,
                                         state,
                                         0,
                                         MAX_POPDATA_VBK,
                                         [](ReadStream& stream,
                                            AltEndorsement::id_t& o,
                                            ValidationState& state) -> bool {
                                           return readSingleByteLenValue(
                                               stream,
                                               o,
                                               state,
                                               AltEndorsement::id_t::size(),
                                               AltEndorsement::id_t::size());
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
