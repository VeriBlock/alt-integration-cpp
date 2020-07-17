// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>

namespace altintegration {

template <>
void PopState<VbkEndorsement>::setDirty() {
  static_cast<BlockIndex<VbkBlock>*>(this)->setDirty();
}

template <>
void PopState<AltEndorsement>::setDirty() {
  static_cast<BlockIndex<AltBlock>*>(this)->setDirty();
}

}  // namespace altintegration
