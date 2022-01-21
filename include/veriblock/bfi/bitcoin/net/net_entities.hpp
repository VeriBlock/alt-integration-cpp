// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_BITCOIN_NET_NET_ENTITIES_HPP
#define BFI_BITCOIN_NET_NET_ENTITIES_HPP

#include <veriblock/bfi/bitcoin/block.hpp>
#include <veriblock/bfi/bitcoin/serialize.hpp>

namespace altintegration {

namespace btc {

/** nServices flags */
enum ServiceFlags : uint64_t {
  // NOTE: When adding here, be sure to update qt/guiutil.cpp's
  // formatServicesStr too
  // Nothing
  NODE_NONE = 0,
  // NODE_NETWORK means that the node is capable of serving the complete block
  // chain. It is currently
  // set by all Bitcoin Core non pruned nodes, and is unset by SPV clients or
  // other light clients.
  NODE_NETWORK = (1 << 0),
  // NODE_GETUTXO means the node is capable of responding to the getutxo
  // protocol request.
  // Bitcoin Core does not support this but a patch set called Bitcoin XT does.
  // See BIP 64 for details on how this is implemented.
  NODE_GETUTXO = (1 << 1),
  // NODE_BLOOM means the node is capable and willing to handle bloom-filtered
  // connections.
  // Bitcoin Core nodes used to support this by default, without advertising
  // this bit,
  // but no longer do as of protocol version 70011 (= NO_BLOOM_VERSION)
  NODE_BLOOM = (1 << 2),
  // NODE_WITNESS indicates that a node can be asked for blocks and transactions
  // including
  // witness data.
  NODE_WITNESS = (1 << 3),
  // NODE_NETWORK_LIMITED means the same as NODE_NETWORK with the limitation of
  // only
  // serving the last 288 (2 day) blocks
  // See BIP159 for details on how this is implemented.
  NODE_NETWORK_LIMITED = (1 << 10),

  // Bits 24-31 are reserved for temporary experiments. Just pick a bit that
  // isn't getting used, or one not being used much, and notify the
  // bitcoin-development mailing list. Remember that service bits are just
  // unauthenticated advertisements, so your code must be robust against
  // collisions and other cases where nodes may be advertising a service they
  // do not actually support. Other service bits should be allocated via the
  // BIP process.
};

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

struct Address : public Service {
  ServiceFlags nServices;
  // disk and network only
  uint32_t nTime;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    uint64_t nServicesInt = nServices;
    READWRITE(nTime);
    READWRITE(nServicesInt);
    nServices = static_cast<ServiceFlags>(nServicesInt);
    READWRITEAS(Service, *this);
  }

  friend bool operator==(const Address& a, const Address& b) {
    return a.nTime == b.nTime && a.nTime == b.nTime && (Service)a == (Service)b;
  }
  friend bool operator!=(const Address& a, const Address& b) {
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

struct BlockTransactionsRequest {
  uint256 blockhash;
  std::vector<uint16_t> indexes;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->blockhash);
    uint64_t indexes_size = (uint64_t)this->indexes.size();
    READWRITE(COMPACTSIZE(indexes_size));
    if (ser_action.ForRead()) {
      size_t i = 0;
      while (indexes.size() < indexes_size) {
        this->indexes.resize(
            std::min((uint64_t)(1000 + this->indexes.size()), indexes_size));
        for (; i < this->indexes.size(); ++i) {
          uint64_t index = 0;
          READWRITE(COMPACTSIZE(index));
          if (index > std::numeric_limits<uint16_t>::max()) {
            throw std::ios_base::failure("index overflowed 16 bits");
          }
          this->indexes[i] = (uint16_t)index;
        }
      }

      int32_t offset = 0;
      for (size_t j = 0; j < this->indexes.size(); j++) {
        if (int32_t(this->indexes[j]) + offset >
            std::numeric_limits<uint16_t>::max())
          throw std::ios_base::failure("indexes overflowed 16 bits");
        this->indexes[j] = this->indexes[j] + (uint16_t)offset;
        offset = int32_t(this->indexes[j]) + 1;
      }
    } else {
      for (size_t i = 0; i < this->indexes.size(); i++) {
        uint64_t index =
            this->indexes[i] - (i == 0 ? 0 : (this->indexes[i - 1] + 1));
        READWRITE(COMPACTSIZE(index));
      }
    }
  }

  friend bool operator==(const BlockTransactionsRequest& a,
                         const BlockTransactionsRequest& b) {
    return a.blockhash == b.blockhash && a.indexes == b.indexes;
  }
  friend bool operator!=(const BlockTransactionsRequest& a,
                         const BlockTransactionsRequest& b) {
    return !(a == b);
  }
};

// Dumb serialization/storage-helper for CBlockHeaderAndShortTxIDs and
// PartiallyDownloadedBlock
struct PrefilledTransaction {
  // Used as an offset since last prefilled tx in BlockHeaderAndShortTxIDs,
  // as a proper transaction-in-block-index in PartiallyDownloadedBlock
  uint16_t index;
  Transaction tx;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    uint64_t idx = this->index;
    READWRITE(COMPACTSIZE(idx));
    if (idx > std::numeric_limits<uint16_t>::max()) {
      throw std::ios_base::failure("index overflowed 16-bits");
    }
    index = (uint16_t)idx;
    READWRITE(this->tx);
  }

  friend bool operator==(const PrefilledTransaction& a,
                         const PrefilledTransaction& b) {
    return a.index == b.index && a.tx == b.tx;
  }
  friend bool operator!=(const PrefilledTransaction& a,
                         const PrefilledTransaction& b) {
    return !(a == b);
  }
};

struct BlockHeaderAndShortTxIDs {
  static const uint32_t SHORTTXIDS_LENGTH = 6;

  BlockHeader header;
  uint64_t nonce;
  std::vector<uint64_t> shorttxids;
  std::vector<PrefilledTransaction> prefilledtxn;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->header);
    READWRITE(this->nonce);

    uint64_t shorttxids_size = (uint64_t)this->shorttxids.size();
    READWRITE(COMPACTSIZE(shorttxids_size));
    if (ser_action.ForRead()) {
      size_t i = 0;
      while (this->shorttxids.size() < shorttxids_size) {
        this->shorttxids.resize(std::min(
            (uint64_t)(1000 + this->shorttxids.size()), shorttxids_size));
        for (; i < shorttxids.size(); i++) {
          uint32_t lsb = 0;
          uint16_t msb = 0;
          READWRITE(lsb);
          READWRITE(msb);
          this->shorttxids[i] = (uint64_t(msb) << 32) | uint64_t(lsb);
        }
      }
    } else {
      for (size_t i = 0; i < this->shorttxids.size(); i++) {
        uint32_t lsb = this->shorttxids[i] & 0xffffffff;
        uint16_t msb = (this->shorttxids[i] >> 32) & 0xffff;
        READWRITE(lsb);
        READWRITE(msb);
      }
    }

    READWRITE(this->prefilledtxn);

    if (this->shorttxids.size() + this->prefilledtxn.size() >
        std::numeric_limits<uint16_t>::max()) {
      throw std::ios_base::failure("indexes overflowed 16 bits");
    }
  }

  friend bool operator==(const BlockHeaderAndShortTxIDs& a,
                         const BlockHeaderAndShortTxIDs& b) {
    return a.header == b.header && a.nonce == b.nonce &&
           a.shorttxids == b.shorttxids && a.prefilledtxn == b.prefilledtxn;
  }
  friend bool operator!=(const BlockHeaderAndShortTxIDs& a,
                         const BlockHeaderAndShortTxIDs& b) {
    return !(a == b);
  }
};

struct BlockTransactions {
  // A BlockTransactions message
  uint256 blockhash;
  std::vector<Transaction> txn;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->blockhash);
    uint64_t txn_size = (uint64_t)this->txn.size();
    READWRITE(COMPACTSIZE(txn_size));
    if (ser_action.ForRead()) {
      size_t i = 0;
      while (this->txn.size() < txn_size) {
        this->txn.resize(
            std::min((uint64_t)(1000 + this->txn.size()), txn_size));
        for (; i < this->txn.size(); i++) {
          READWRITE(this->txn[i]);
        }
      }
    } else {
      for (size_t i = 0; i < txn.size(); i++) {
        READWRITE(this->txn[i]);
      }
    }
  }

  friend bool operator==(const BlockTransactions& a,
                         const BlockTransactions& b) {
    return a.blockhash == b.blockhash && a.txn == b.txn;
  }
  friend bool operator!=(const BlockTransactions& a,
                         const BlockTransactions& b) {
    return !(a == b);
  }
};

struct BloomFilter {
  std::vector<uint8_t> vData;
  uint32_t nHashFuncs;
  uint32_t nTweak;
  uint8_t nFlags;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->vData);
    READWRITE(this->nHashFuncs);
    READWRITE(this->nTweak);
    READWRITE(this->nFlags);
  }

  friend bool operator==(const BloomFilter& a, const BloomFilter& b) {
    return a.vData == b.vData && a.nHashFuncs == b.nHashFuncs &&
           a.nTweak == b.nTweak && a.nFlags == b.nFlags;
  }
  friend bool operator!=(const BloomFilter& a, const BloomFilter& b) {
    return !(a == b);
  }
};

}  // namespace btc

}  // namespace altintegration

#endif