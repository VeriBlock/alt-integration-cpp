// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/vbk_block_addon.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

template <>
template <>
const std::vector<typename VTB::id_t>&
VbkBlockAddon<BlockIndex<VbkBlock>>::getPayloadIds<VTB>() const {
  return _vtbids;
}

}  // namespace altintegration
