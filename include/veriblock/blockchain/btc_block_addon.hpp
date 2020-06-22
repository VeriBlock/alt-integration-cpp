// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

#include <veriblock/arith_uint256.hpp>

namespace altintegration {

struct BtcBlockAddon {
  //! total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! reference counter for fork resolution
  uint32_t refCounter = 0;

  void setNull() {
    refCounter = 0;
    chainWork = 0;
  }
};

}

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP
