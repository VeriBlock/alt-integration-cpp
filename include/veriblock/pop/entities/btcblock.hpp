// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/blockchain/btc_block_addon.hpp>
#include <veriblock/pop/fmt.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/storage/stored_btc_block_addon.hpp>
#include <veriblock/pop/uint.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

/**
 * @struct BtcBlock
 *
 * Bitcoin block.
 *
 */
struct BtcBlock {
  using hash_t = uint256;
  using prev_hash_t = uint256;
  using height_t = int32_t;
  using nonce_t = uint32_t;
  using merkle_t = uint256;
  using addon_t = StoredBtcBlockAddon::addon_t;
  using stored_addon_t = StoredBtcBlockAddon;

  BtcBlock() = default;
  BtcBlock(uint32_t version,
           uint256 previousBlock,
           uint256 merkleRoot,
           uint32_t timestamp,
           uint32_t bits,
           uint32_t nonce);

  /**
   * Convert BtcBlock to data stream using BtcBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert BtcBlock to data stream using BtcBlock VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  friend bool operator==(const BtcBlock& a, const BtcBlock& b) {
    // clang-format off
    return a.bits == b.bits &&
           a.version == b.version &&
           a.timestamp == b.timestamp &&
           a.nonce == b.nonce &&
           a.merkleRoot == b.merkleRoot &&
           a.previousBlock == b.previousBlock;
    // clang-format on
  }

  friend bool operator!=(const BtcBlock& a, const BtcBlock& b) {
    return !(a == b);
  }

  hash_t calculateHash() const;

  /**
   * Calculate the hash of the btc block
   * @return hash block hash
   */
  const hash_t& getHash() const;

  static const std::string& name() { return _name; }

  std::string toPrettyString() const;

  uint32_t getVersion() const { return version; }
  uint256 getPreviousBlock() const { return previousBlock; }
  uint256 getMerkleRoot() const { return merkleRoot; }
  uint32_t getNonce() const { return nonce; }
  uint32_t getTimestamp() const { return timestamp; }
  uint32_t getDifficulty() const { return bits; }

  void setVersion(uint32_t v);
  void setPreviousBlock(const uint256& prev);
  void setMerkleRoot(const uint256& mr);
  void setDifficulty(uint32_t bits);
  void setNonce(uint32_t nnc);
  void setTimestamp(uint32_t ts);

 private:
  static const std::string _name;

  void invalidateHash() const { hash_.fill(0); }

  uint32_t version = 0;
  uint256 previousBlock{};
  uint256 merkleRoot{};
  uint32_t timestamp = 0;
  uint32_t bits = 0;
  uint32_t nonce = 0;

  mutable hash_t hash_;

  friend bool DeserializeFromRaw(ReadStream& stream,
                                 BtcBlock& block,
                                 ValidationState& state,
                                 const BtcBlock::hash_t& precalculatedHash);

  friend bool DeserializeFromVbkEncoding(
      ReadStream& stream,
      BtcBlock& block,
      ValidationState& state,
      const BtcBlock::hash_t& precalculatedHash);
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const BtcBlock& b) {
  JsonValue object = json::makeEmptyObject<JsonValue>();
  json::putStringKV(object, "hash", HexStr(b.getHash()));
  json::putIntKV(object, "version", b.getVersion());
  json::putStringKV(object, "previousBlock", HexStr(b.getPreviousBlock()));
  json::putStringKV(object, "merkleRoot", HexStr(b.getMerkleRoot()));
  json::putIntKV(object, "timestamp", b.getTimestamp());
  json::putIntKV(object, "bits", b.getDifficulty());
  json::putIntKV(object, "nonce", b.getNonce());
  return object;
}

//! @overload
bool DeserializeFromRaw(
    ReadStream& stream,
    BtcBlock& out,
    ValidationState& state,
    const AltChainParams& /*ignore*/,
    const BtcBlock::hash_t& precalculatedHash = BtcBlock::hash_t{});

//! @overload
bool DeserializeFromRaw(
    ReadStream& stream,
    BtcBlock& out,
    ValidationState& state,
    const BtcBlock::hash_t& precalculatedHash = BtcBlock::hash_t{});

//! @overload
bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    BtcBlock& out,
    ValidationState& state,
    const AltChainParams& /*ignore*/,
    const BtcBlock::hash_t& precalculatedHash = BtcBlock::hash_t{});

//! @overload
bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    BtcBlock& out,
    ValidationState& state,
    const BtcBlock::hash_t& precalculatedHash = BtcBlock::hash_t{});

}  // namespace altintegration

//! @private
template <>
struct std::hash<altintegration::BtcBlock> {
  size_t operator()(const altintegration::BtcBlock& block) {
    return altintegration::ArithUint256(block.getHash()).getLow64();
  }
};

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
