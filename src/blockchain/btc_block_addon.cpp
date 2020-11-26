// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

void BtcBlockAddon::setDirty() {
  static_cast<BlockIndex<BtcBlock>*>(this)->setDirty();
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<int32_t>(stream,
                            out.refs,
                            state,
                            0,
                            MAX_BTCADDON_REFS,
                            [&](int32_t& out) -> bool {
                              return stream.readBE<int32_t>(out, state);
                            })) {
    return state.Invalid("bad-refs");
  }

  return true;
}

}  // namespace altintegration
