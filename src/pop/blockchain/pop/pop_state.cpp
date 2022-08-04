// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/blockchain/pop/pop_state.hpp"

#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/entities/altblock.hpp"
#include "veriblock/pop/entities/endorsements.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"

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
