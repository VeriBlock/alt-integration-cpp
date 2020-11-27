// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/vbk_block_addon.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/fmt.hpp>
#include <veriblock/hashutil.hpp>
#include <veriblock/serde.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VbkChainParams;

/**
 * @struct VbkBlock
 *
 * Veriblock block
 */
struct VbkBlock {
  using hash_t = uint192;
  using short_hash_t = uint96;
  using id_t = short_hash_t;
  using prev_hash_t = short_hash_t;
  using height_t = int32_t;
  using nonce_t = int32_t;
  using keystone_t = uint72;
  using merkle_t = uint128;
  using protecting_block_t = BtcBlock;
  using addon_t = VbkBlockAddon;

  std::string toPrettyString() const;

  /**
   * Convert VbkBlock to data stream using VbkBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkBlock to raw bytes data using VbkBlock byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Convert VbkBlock to bytes data using VbkBlock basic byte format
   * @return string represantation of the data
   */
  std::vector<uint8_t> toRaw() const;

  /**
   * Convert VbkBlock to data stream using VbkBlock VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /*
   * Getter for difficulty
   * @return block difficulty
   */
  uint32_t getDifficulty() const;

  size_t estimateSize() const { return toVbkEncoding().size(); }

  /*
   * Getter for timestamp
   * @return block timestamp
   */
  uint32_t getBlockTime() const;

  friend bool operator==(const VbkBlock& a, const VbkBlock& b);
  friend bool operator!=(const VbkBlock& a, const VbkBlock& b);

  hash_t calculateHash() const;

  /**
   * Get current block hash
   * @return hash block hash
   */
  hash_t getHash() const;

  /**
   * Calculate the hash of the vbk block
   * @return hash block hash
   */
  short_hash_t getShortHash() const;

  short_hash_t getId() const { return getShortHash(); }

  static const std::string& name() { return _name; }

  VbkBlock() = default;
  VbkBlock(int32_t h,
           int16_t v,
           uint96 prevBlock,
           keystone_t prev1,
           keystone_t prev2,
           uint128 mroot,
           int32_t ts,
           int32_t diff,
           uint64_t nonce);

  int32_t getHeight() const { return height; }
  int16_t getVersion() const { return version; }
  uint96 getPreviousBlock() const { return previousBlock; }
  keystone_t getPreviousKeystone() const { return previousKeystone; }
  keystone_t getSecondPreviousKeystone() const {
    return secondPreviousKeystone;
  }
  uint128 getMerkleRoot() const { return merkleRoot; }
  int32_t getTimestamp() const { return timestamp; }
  uint64_t getNonce() const { return nonce; }

  void setHeight(int32_t h);
  void setVersion(int16_t v);
  void setPreviousBlock(const uint96& prev);
  void setPreviousKeystone(const keystone_t& ks);
  void setSecondPreviousKeystone(const keystone_t& ks);
  void setMerkleRoot(const uint128& mroot);
  void setTimestamp(int32_t ts);
  void setDifficulty(int32_t diff);
  void setNonce(uint64_t nnc);

 private:
  static const std::string _name;

  int32_t height{};
  int16_t version{};
  uint96 previousBlock{};
  keystone_t previousKeystone{};
  keystone_t secondPreviousKeystone{};
  uint128 merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  uint64_t nonce{};

  mutable hash_t hash_{};

  void invalidateHash() { hash_.fill(0); }

  friend bool DeserializeFromRaw(ReadStream& stream,
                                 VbkBlock& out,
                                 ValidationState& state,
                                 const VbkBlock::hash_t& precalculatedHash);
};

template <typename JsonValue>
JsonValue ToJSON(const VbkBlock& b) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", HexStr(b.getId()));
  json::putStringKV(obj, "hash", HexStr(b.getHash()));
  json::putIntKV(obj, "height", b.getHeight());
  json::putIntKV(obj, "version", b.getVersion());
  json::putStringKV(obj, "previousBlock", HexStr(b.getPreviousBlock()));
  json::putStringKV(obj, "previousKeystone", HexStr(b.getPreviousKeystone()));
  json::putStringKV(
      obj, "secondPreviousKeystone", HexStr(b.getSecondPreviousKeystone()));
  json::putStringKV(obj, "merkleRoot", HexStr(b.getMerkleRoot()));
  json::putIntKV(obj, "timestamp", b.getBlockTime());
  json::putIntKV(obj, "difficulty", b.getDifficulty());
  json::putIntKV(obj, "nonce", b.getNonce());
  return obj;
}

bool DeserializeFromRaw(ReadStream& stream,
                        VbkBlock& out,
                        ValidationState& state,
                        const VbkBlock::hash_t& hash = VbkBlock::hash_t{});

bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    VbkBlock& out,
    ValidationState& state,
    const VbkBlock::hash_t& hash = VbkBlock::hash_t{});

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
