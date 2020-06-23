// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VTB;

struct VbkBlockAddon :
    // for endorsement map
    public PopState<VbkEndorsement>,
    // for chainwork + ref
    public BtcBlockAddon {
  using payloads_t = VTB;

  // VTB::id_t
  std::vector<uint256> payloadIds;

  bool payloadsIdsEmpty() const { return payloadIds.empty(); }

  void setNull() {
    BtcBlockAddon::setNull();
    PopState<VbkEndorsement>::setNull();
    payloadIds.clear();
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
