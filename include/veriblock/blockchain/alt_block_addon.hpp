// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP

#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct AltBlockAddon : public PopState<AltEndorsement> {
  std::vector<uint256> payloadIds;

  // TODO: refactor base block tree, and move chainwork to blocktree.hpp, then
  // remove this
  uint8_t chainWork;

  void setNull() {
    PopState<AltEndorsement>::setNull();
    chainWork = 0;
    payloadIds.clear();
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
