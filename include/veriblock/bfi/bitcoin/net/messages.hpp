// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_BITCOIN_NET_MESSAGES_HPP
#define BFI_BITCOIN_NET_MESSAGES_HPP

#include <veriblock/bfi/bitcoin/block.hpp>
#include <veriblock/bfi/bitcoin/net/net_entities.hpp>
#include <veriblock/bfi/bitcoin/serialize.hpp>

namespace altintegration {

namespace btc {

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

struct GetBlockTxnMsg {
  BlockTransactionsRequest req;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->req);
  }

  friend bool operator==(const GetBlockTxnMsg& a, const GetBlockTxnMsg& b) {
    return a.req == b.req;
  }
  friend bool operator!=(const GetBlockTxnMsg& a, const GetBlockTxnMsg& b) {
    return !(a == b);
  }
};

struct GetHeadersMsg {
  BlockLocator locator;
  uint256 hashStop;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->locator);
    READWRITE(this->hashStop);
  }

  friend bool operator==(const GetHeadersMsg& a, const GetHeadersMsg& b) {
    return a.locator == b.locator && a.hashStop == b.hashStop;
  }
  friend bool operator!=(const GetHeadersMsg& a, const GetHeadersMsg& b) {
    return !(a == b);
  }
};

struct TxMsg {
  Transaction tx;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->tx);
  }

  friend bool operator==(const TxMsg& a, const TxMsg& b) {
    return a.tx == b.tx;
  }
  friend bool operator!=(const TxMsg& a, const TxMsg& b) { return !(a == b); }
};

struct CmpctBlockMsg {
  BlockHeaderAndShortTxIDs block_header;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->block_header);
  }

  friend bool operator==(const CmpctBlockMsg& a, const CmpctBlockMsg& b) {
    return a.block_header == b.block_header;
  }
  friend bool operator!=(const CmpctBlockMsg& a, const CmpctBlockMsg& b) {
    return !(a == b);
  }
};

struct BlockTxnMsg {
  BlockTransactions resp;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->resp);
  }

  friend bool operator==(const BlockTxnMsg& a, const BlockTxnMsg& b) {
    return a.resp == b.resp;
  }
  friend bool operator!=(const BlockTxnMsg& a, const BlockTxnMsg& b) {
    return !(a == b);
  }
};

}  // namespace btc

}  // namespace altintegration

#endif