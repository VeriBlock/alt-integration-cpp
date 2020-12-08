// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/alt_block_addon.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>

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
          [&](uint256& o) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("alt-blocka-addon-bad-atvid");
  }

  if (!readArrayOf<uint256>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_POPDATA_VTB,
          [&](uint256& o) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("alt-blocka-addon-bad-vtbid");
  }

  if (!readArrayOf<uint96>(
          stream,
          out._vbkblockids,
          state,
          0,
          MAX_POPDATA_VBK,
          [&](uint96& o) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint96::size(), uint96::size());
          })) {
    return state.Invalid("alt-blocka-addon-bad-vbkids");
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
  return fmt::sprintf("ATV=%d, VTB=%d, VBK=%d",
                      _atvids.size(),
                      _vtbids.size(),
                      _vbkblockids.size());
}

void AltBlockAddon::toVbkEncoding(WriteStream& w) const {
  PopState<AltEndorsement>::toVbkEncoding(w);
  writeArrayOf<uint256>(w, _atvids, writeSingleByteLenValue);
  writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  writeArrayOf<uint96>(w, _vbkblockids, writeSingleByteLenValue);
}

void AltBlockAddon::setNullInmemFields() {
  endorsedBy.clear();
}

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
