// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/bfi/bitcoin/block.hpp>
#include <veriblock/bfi/bitcoin/net/net_entities.hpp>
#include <veriblock/bfi/bitcoin/serialize.hpp>

namespace altintegration {

namespace btc {

struct GetBlocksMsg {
  BlockLocator locator;
  uint256 hashStop;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->locator);
    READWRITE(this->hashStop);
  }

  friend bool operator==(const GetBlocksMsg& a, const GetBlocksMsg& b) {
    return a.locator == b.locator && a.hashStop == b.hashStop;
  }
  friend bool operator!=(const GetBlocksMsg& a, const GetBlocksMsg& b) {
    return !(a == b);
  }
};

struct InvMsg {
  std::vector<Inv> vInv;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->vInv);
  }

  friend bool operator==(const InvMsg& a, const InvMsg& b) {
    return a.vInv == b.vInv;
  }
  friend bool operator!=(const InvMsg& a, const InvMsg& b) { return !(a == b); }
};

struct GetDataMsg {
  std::vector<Inv> vInv;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->vInv);
  }

  friend bool operator==(const GetDataMsg& a, const GetDataMsg& b) {
    return a.vInv == b.vInv;
  }
  friend bool operator!=(const GetDataMsg& a, const GetDataMsg& b) {
    return !(a == b);
  }
};

// struct GetBlockTxnMsg {
//   ADD_SERIALIZE_METHODS;

//   template <typename Stream, typename Operation>
//   inline void SerializationOp(Stream& s, Operation ser_action) {
//     READWRITE(this->vInv);
//   }

//   friend bool operator==(const GetDataMsg& a, const GetDataMsg& b) {
//     return a.vInv == b.vInv;
//   }
//   friend bool operator!=(const GetDataMsg& a, const GetDataMsg& b) {
//     return !(a == b);
//   }
// };

}  // namespace btc

}  // namespace altintegration