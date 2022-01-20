// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_BITCOIN_BLOCK_HPP
#define BFI_BITCOIN_BLOCK_HPP

#include <veriblock/bfi/bitcoin/serialize.hpp>
#include <veriblock/bfi/bitcoin/transaction.hpp>
#include <veriblock/pop/entities/btcblock.hpp>

namespace altintegration {

namespace btc {

struct BlockHeader : public BtcBlock {
  BlockHeader() = default;

  BlockHeader(int32_t version,
              uint256 previousBlock,
              uint256 merkleRoot,
              uint32_t timestamp,
              uint32_t bits,
              uint32_t nonce)
      : BtcBlock(version, previousBlock, merkleRoot, timestamp, bits, nonce) {}

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->version);
    READWRITE(this->previousBlock);
    READWRITE(this->merkleRoot);
    READWRITE(this->timestamp);
    READWRITE(this->bits);
    READWRITE(this->nonce);
  }
};

struct Block : public BlockHeader {
  std::vector<Transaction> vtx{};

  Block() = default;

  Block(int32_t version,
        uint256 previousBlock,
        uint256 merkleRoot,
        uint32_t timestamp,
        uint32_t bits,
        uint32_t nonce)
      : BlockHeader(
            version, previousBlock, merkleRoot, timestamp, bits, nonce) {}

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITEAS(BlockHeader, *this);
    READWRITE(this->vtx);
  }

  friend bool operator==(const Block& a, const Block& b) {
    return (BlockHeader)a == (BlockHeader)b && a.vtx == b.vtx;
  }

  friend bool operator!=(const Block& a, const Block& b) { return !(a == b); }
};

struct BlockLocator {
  std::vector<uint256> vHave;

  BlockLocator() = default;

  BlockLocator(const std::vector<uint256>& vHave) : vHave(vHave) {}

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    int nVersion = s.getVersion();
    READWRITE(nVersion);
    READWRITE(vHave);
  }

  friend bool operator==(const BlockLocator& a, const BlockLocator& b) {
    return a.vHave == b.vHave;
  }

  friend bool operator!=(const BlockLocator& a, const BlockLocator& b) {
    return !(a == b);
  }
};

}  // namespace btc

}  // namespace altintegration

#endif