// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/facade/btc_block_tree_facade.hpp>

namespace altintegration {

bool BtcBlockTreeFacade::acceptBlockHeader(std::shared_ptr<BtcBlock> header,
                                           ValidationState& state) {
  return btc_->acceptBlockHeader(std::move(header), state);
}

}  // namespace altintegration