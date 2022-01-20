// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/alt_block_addon.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>

namespace altintegration {

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AltBlockAddon& out,
                                ValidationState& state) {
  PopState<AltEndorsement>& pop = out;
  if (!DeserializeFromVbkEncoding(stream, pop, state)) {
    return state.Invalid("alt-block-addon-bad-popstate");
  }

  if (!readArrayOf<uint256>(
          stream,
          out._atvids,
          state,
          0,
          MAX_POPDATA_ATV,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("alt-block-addon-bad-atvid");
  }

  if (!readArrayOf<uint256>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_POPDATA_VTB,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("alt-block-addon-bad-vtbid");
  }

  if (!readArrayOf<uint96>(
          stream,
          out._vbkblockids,
          state,
          0,
          MAX_POPDATA_VBK,
          [](ReadStream& stream, uint96& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint96::size(), uint96::size());
          })) {
    return state.Invalid("alt-block-addon-bad-vbkids");
  }

  return true;
}

void AltBlockAddon::setDirty() {
  static_cast<BlockIndex<AltBlock>*>(this)->setDirty();
}

template <>
const std::vector<typename ATV::id_t>& AltBlockAddon::getPayloadIds<ATV>()
    const {
  return _atvids;
}

template <>
const std::vector<typename VTB::id_t>& AltBlockAddon::getPayloadIds<VTB>()
    const {
  return _vtbids;
}

template <>
const std::vector<typename VbkBlock::id_t>&
AltBlockAddon::getPayloadIds<VbkBlock>() const {
  return _vbkblockids;
}

template <>
std::vector<typename ATV::id_t>& AltBlockAddon::getPayloadIdsInner<ATV>() {
  return _atvids;
}

std::string AltBlockAddon::toPrettyString() const {
  return altintegration::format("ATVs={}[{}], VTBs={}[{}], VBKs={}[{}]",
                                _atvids.size(),
                                fmt::join(_atvids, ","),
                                _vtbids.size(),
                                fmt::join(_vtbids, ","),
                                _vbkblockids.size(),
                                fmt::join(_vbkblockids, ","));
}

void AltBlockAddon::toVbkEncoding(WriteStream& w) const {
  PopState<AltEndorsement>::toVbkEncoding(w);
  writeArrayOf<uint256>(w, _atvids, writeSingleByteLenValue);
  writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  writeArrayOf<uint96>(w, _vbkblockids, writeSingleByteLenValue);
}

void AltBlockAddon::setNullInmemFields() { _endorsedBy.clear(); }

bool AltBlockAddon::hasPayloads() const {
  return !_atvids.empty() || !_vtbids.empty() || !_vbkblockids.empty();
}

void AltBlockAddon::clearPayloads() {
  _atvids.clear();
  _vtbids.clear();
  _vbkblockids.clear();
}

void AltBlockAddon::setNull() {
  PopState<AltEndorsement>::setNull();
  clearPayloads();
}

template <>
std::vector<typename VTB::id_t>& AltBlockAddon::getPayloadIdsInner<VTB>() {
  return _vtbids;
}

template <>
std::vector<typename VbkBlock::id_t>&
AltBlockAddon::getPayloadIdsInner<VbkBlock>() {
  return _vbkblockids;
}

}  // namespace altintegration
