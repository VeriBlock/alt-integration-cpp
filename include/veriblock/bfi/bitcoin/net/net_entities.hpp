// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/bfi/bitcoin/serialize.hpp>

namespace altintegration {

namespace btc {

/** IP address (IPv6, or IPv4 using mapped IPv6 range */
struct NetAddr {
  // in network byte order
  uint8_t ip[16];

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->ip);
  }

  friend bool operator==(const NetAddr& a, const NetAddr& b) {
    return (memcmp(a.ip, b.ip, 16) == 0);
  }
  friend bool operator!=(const NetAddr& a, const NetAddr& b) {
    return !(a == b);
  }
};

struct SubNet {
  /// Network (base) address
  NetAddr network;
  /// Netmask, in network byte order
  uint8_t netmask[16];
  /// Is this value valid? (only used to signal parse errors)
  bool valid;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->network);
    READWRITE(this->netmask);
    READWRITE(this->valid);
  }

  friend bool operator==(const SubNet& a, const SubNet& b) {
    return a.valid == b.valid && a.network == b.network &&
           !memcmp(a.netmask, b.netmask, 16);
  }
  friend bool operator!=(const SubNet& a, const SubNet& b) { return !(a == b); }
};

/** A combination of a network address (CNetAddr) and a (TCP) port */
struct Service : public NetAddr {
  uint16_t port;  // host order

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITEAS(NetAddr, *this);
    READWRITE(WrapBigEndian(this->port));
  }

  friend bool operator==(const Service& a, const Service& b) {
    return a.port == b.port && (NetAddr)a == (NetAddr)b;
  }
  friend bool operator!=(const Service& a, const Service& b) {
    return !(a == b);
  }
};

struct BanEntry {
  int32_t nVersion;
  int64_t nCreateTime;
  int64_t nBanUntil;
  uint8_t banReason;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->nVersion);
    READWRITE(this->nCreateTime);
    READWRITE(this->nBanUntil);
    READWRITE(this->banReason);
  }

  friend bool operator==(const BanEntry& a, const BanEntry& b) {
    return a.nVersion == b.nVersion && a.nCreateTime == b.nCreateTime &&
           a.nBanUntil == b.nBanUntil && a.banReason == b.banReason;
  }
  friend bool operator!=(const BanEntry& a, const BanEntry& b) {
    return !(a == b);
  }
};

struct Inv {
  int32_t type;
  uint256 hash;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->type);
    READWRITE(this->hash);
  }

  friend bool operator==(const Inv& a, const Inv& b) {
    return a.type == b.type && a.hash == b.hash;
  }
  friend bool operator!=(const Inv& a, const Inv& b) { return !(a == b); }
};

struct BlockTransactionRequest {
  uint256 blockhash;
  std::vector<uint16_t> indexes;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->blockhash);
    // uint64_t indexes_size = (uint64_t)indexes.size();
  }
};

}  // namespace btc

}  // namespace altintegration
