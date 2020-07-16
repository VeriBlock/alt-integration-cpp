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

template <>
template <>
const std::vector<typename ATV::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIds<ATV>() const {
  return _atvids;
}

template <>
template <>
const std::vector<typename VTB::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIds<VTB>() const {
  return _vtbids;
}

template <>
template <>
const std::vector<typename VbkBlock::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIds<VbkBlock>() const {
  return _vbkblockids;
}

template <>
template <>
std::vector<typename ATV::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIdsInner<ATV>() {
  return _atvids;
}

template <>
template <>
std::vector<typename VTB::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIdsInner<VTB>() {
  return _vtbids;
}

template <>
template <>
std::vector<typename VbkBlock::id_t>&
AltBlockAddon<BlockIndex<AltBlock>>::getPayloadIdsInner<VbkBlock>() {
  return _vbkblockids;
}

}  // namespace altintegration
