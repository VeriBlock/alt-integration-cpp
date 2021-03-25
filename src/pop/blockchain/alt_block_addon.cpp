// Copyright (c) 2019-2020 Xenios SEZC
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
  return fmt::format("ATVs={}[{}], VTBs={}[{}], VBKs={}[{}]",
                     _atvids.size(),
                     fmt::join(_atvids, ","),
                     _vtbids.size(),
                     fmt::join(_vtbids, ","),
                     _vbkblockids.size(),
                     fmt::join(_vbkblockids, ","));
}

void AltBlockAddon::setNullInmemFields() { endorsedBy.clear(); }

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
