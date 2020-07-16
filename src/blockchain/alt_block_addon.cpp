// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/alt_block_addon.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

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
