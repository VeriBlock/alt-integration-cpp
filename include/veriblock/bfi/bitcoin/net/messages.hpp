// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/bfi/bitcoin/block.hpp>
#include <veriblock/bfi/bitcoin/serialize.hpp>

namespace altintegration {

namespace btc {

struct GetBlocks {
  BlockLocator locator;
  uint256 hashStop;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->locator);
    READWRITE(this->hashStop);
  }

  friend bool operator==(const GetBlocks& a, const GetBlocks& b) {
    return a.locator == b.locator && a.hashStop == b.hashStop;
  }
  friend bool operator!=(const GetBlocks& a, const GetBlocks& b) {
    return !(a == b);
  }
};

}  // namespace btc

}  // namespace altintegration