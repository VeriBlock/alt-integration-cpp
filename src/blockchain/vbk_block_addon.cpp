// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/vbk_block_addon.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

void VbkBlockAddon::setDirty() {
  static_cast<BlockIndex<VbkBlock>*>(this)->setDirty();
}

template <>
const std::vector<typename VTB::id_t>& VbkBlockAddon::getPayloadIds<VTB>()
    const {
  return _vtbids;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkBlockAddon& out,
                                ValidationState& state) {
  if (!stream.readBE(out._refCount, state)) {
    return state.Invalid("vbk-addon-bad-ref-count");
  }

  PopState<VbkEndorsement>& pop = out;
  if (!DeserializeFromVbkEncoding(stream, pop, state)) {
    return state.Invalid("vbk-addon-bad-popstate");
  }

  if (!readArrayOf<uint256>(
          stream,
          out._vtbids,
          state,
          0,
          MAX_VBKPOPTX_PER_VBK_BLOCK,
          [&](uint256& o) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("vbk-addon-bad-vtbid");
  }

  return true;
}

}  // namespace altintegration
