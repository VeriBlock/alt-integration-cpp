// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/vbk_block_addon.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/endorsements.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct VbkBlockAddon;

struct VbkBlock {
  using hash_t = uint192;
  using short_hash_t = uint96;
  using id_t = short_hash_t;
  using prev_hash_t = short_hash_t;
  using height_t = int32_t;
  using keystone_t = uint72;
  using protecting_block_t = BtcBlock;
  using addon_t = VbkBlockAddon;

  int32_t height{};
  int16_t version{};
  uint96 previousBlock{};
  keystone_t previousKeystone{};
  keystone_t secondPreviousKeystone{};
  uint128 merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  int32_t nonce{};

  // TODO: remove on command group cache implemented
  // flag to mark invalid command groups.
  bool valid{};

  std::string toPrettyString() const;

  static VbkBlock fromHex(const std::string& hex);

  /**
   * Read basic blockheader data from the vector of bytes and convert it to
   * VbkBlock
   * @param bytes data to read fromm
   * @return VbkBlock
   */
  static VbkBlock fromRaw(Slice<const uint8_t> bytes);

  /**
   * Read basic blockheader data from the stream and convert it to VbkBlock
   * @param stream data stream to read from
   * @return VbkBlock
   */
  static VbkBlock fromRaw(ReadStream& stream);

  /**
   * Read VBK data from the stream and convert it to VbkBlock
   * @param stream data stream to read from
   * @return VbkBlock
   */
  static VbkBlock fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VbkBlock to data stream using VbkBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkBlock to bytes data using VbkBlock basic byte format
   * @return string represantation of the data
   */
  std::vector<uint8_t> toRaw() const;

  /**
   * Convert VbkBlock to Hex string using VbkBlock basic byte format
   * @return Hex represantation of the data
   */
  std::string toHex() const;

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

  /*
   * Getter for timestamp
   * @return block timestamp
   */
  uint32_t getBlockTime() const;

  friend bool operator==(const VbkBlock& a, const VbkBlock& b) {
    return a.getHash() == b.getHash();
  }

  friend bool operator!=(const VbkBlock& a, const VbkBlock& b) {
    return !(a == b);
  }

  /**
   * Calculate the hash of the vbk block
   * @return hash block hash
   */
  hash_t getHash() const;

  /**
   * Calculate the hash of the vbk block
   * @return hash block hash
   */
  short_hash_t getShortHash() const;

  short_hash_t getId() const { return getShortHash(); }

  static std::string name() { return "VBK"; }
};

template <typename JsonValue>
JsonValue ToJSON(const VbkBlock& b) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", HexStr(b.getId()));
  json::putStringKV(obj, "hash", HexStr(b.getHash()));
  json::putIntKV(obj, "height", b.height);
  json::putIntKV(obj, "version", b.version);
  json::putStringKV(obj, "previousBlock", HexStr(b.previousBlock));
  json::putStringKV(obj, "previousKeystone", HexStr(b.previousKeystone));
  json::putStringKV(
      obj, "secondPreviousKeystone", HexStr(b.secondPreviousKeystone));
  json::putStringKV(obj, "merkleRoot", HexStr(b.merkleRoot));
  json::putIntKV(obj, "timestamp", b.timestamp);
  json::putIntKV(obj, "difficulty", b.difficulty);
  json::putIntKV(obj, "nonce", b.nonce);
  return obj;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
