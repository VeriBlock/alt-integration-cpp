// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <string>
#include <vector>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/storage/stored_alt_block_addon.hpp>

#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/blockchain/pop/pop_state.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/entities/endorsement.hpp"
#include "veriblock/pop/entities/endorsements.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {
class WriteStream;
struct ReadStream;

StoredAltBlockAddon::StoredAltBlockAddon(const addon_t& other) {
  endorsedByIds =
      map_get_id_from_pointers<AltEndorsement::id_t, const AltEndorsement>(
          other.getEndorsedBy());
  _atvids = other.getPayloadIds<ATV>();
  _vtbids = other.getPayloadIds<VTB>();
  _vbkblockids = other.getPayloadIds<VbkBlock>();
  popState = other;
}

void StoredAltBlockAddon::toVbkEncoding(WriteStream& w) const {
  writeContainer(w, endorsedByIds, writeSingleByteLenValue);
  writeContainer(w, _atvids, writeSingleByteLenValue);
  writeContainer(w, _vtbids, writeSingleByteLenValue);
  writeContainer(w, _vbkblockids, writeSingleByteLenValue);
  popState.toVbkEncoding(w);
}

void StoredAltBlockAddon::toInmem(StoredAltBlockAddon::addon_t& to) const {
  to.template setPayloads<ATV>(_atvids);
  to.template setPayloads<VTB>(_vtbids);
  to.template setPayloads<VbkBlock>(_vbkblockids);
  auto& p = static_cast<pop_state_t&>(to);
  p = popState;
}

std::string StoredAltBlockAddon::toPrettyString() const {
  return altintegration::format("ATVs={}[{}], VTBs={}[{}], VBKs={}[{}]",
                                _atvids.size(),
                                fmt::join(_atvids, ","),
                                _vtbids.size(),
                                fmt::join(_vtbids, ","),
                                _vbkblockids.size(),
                                fmt::join(_vbkblockids, ","));
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredAltBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<AltEndorsement::id_t>(stream,
                                         out.endorsedByIds,
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
    return state.Invalid("stored-alt-block-addon-bad-endorsedby-hash");
  }

  if (!readArrayOf<ATV::id_t>(
          stream,
          out._atvids,
          state,
          0,
          MAX_POPDATA_ATV,
          [](ReadStream& stream, ATV::id_t& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, ATV::id_t::size(), ATV::id_t::size());
          })) {
    return state.Invalid("stored-alt-block-addon-bad-atvid");
  }

  if (!readArrayOf<VTB::id_t>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_POPDATA_VTB,
          [](ReadStream& stream, VTB::id_t& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, VTB::id_t::size(), VTB::id_t::size());
          })) {
    return state.Invalid("stored-alt-block-addon-bad-vtbid");
  }

  if (!readArrayOf<VbkBlock::id_t>(stream,
                                   out._vbkblockids,
                                   state,
                                   0,
                                   MAX_POPDATA_VBK,
                                   [](ReadStream& stream,
                                      VbkBlock::id_t& o,
                                      ValidationState& state) -> bool {
                                     return readSingleByteLenValue(
                                         stream,
                                         o,
                                         state,
                                         VbkBlock::id_t::size(),
                                         VbkBlock::id_t::size());
                                   })) {
    return state.Invalid("stored-alt-block-addon-bad-vbkids");
  }

  if (!DeserializeFromVbkEncoding(stream, out.popState, state)) {
    return state.Invalid("stored-alt-block-addon-bad-popstate");
  }

  return true;
}

}  // namespace altintegration
