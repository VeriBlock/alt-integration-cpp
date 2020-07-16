// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/vbk_block_addon.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

template <>
const std::vector<typename VTB::id_t>& VbkBlockAddon::getPayloadIds<VTB>()
    const {
  return _vtbids;
}

}  // namespace altintegration
