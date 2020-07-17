// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

void BtcBlockAddon::setDirty() { static_cast<BlockIndex<BtcBlock>*>(this)->setDirty(); }

}  // namespace altintegration
