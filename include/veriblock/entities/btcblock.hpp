// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/btc_block_addon.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct BtcBlockAddon;

struct BtcBlock {
  using hash_t = uint256;
  using prev_hash_t = uint256;
  using height_t = int32_t;
  using nonce_t = uint32_t;
  using merkle_t = uint256;
  using addon_t = BtcBlockAddon;

  BtcBlock() = default;

  BtcBlock(uint32_t version,
           uint256 previousBlock,
           uint256 merkleRoot,
           uint32_t timestamp,
           uint32_t bits,
           uint32_t nonce)
      : _version(version),
        _previousBlock(previousBlock),
        _merkleRoot(merkleRoot),
        _timestamp(timestamp),
        _bits(bits),
        _nonce(nonce) {}

  static BtcBlock fromHex(const std::string& hex);

  /**
   * Read basic blockheader data from the vector of bytes and convert it to
   * BtcBlock
   * @param bytes data to read from
   * @return BtcBlock
   */
  static BtcBlock fromRaw(const std::vector<uint8_t>& bytes);

  /**
   * Read basic blockheader data from the stream and convert it to BtcBlock
   * @param stream data to read from
   * @return BtcBlock
   */
  static BtcBlock fromRaw(ReadStream& stream);

  /**
   * Read VBK data from the stream and convert it to BtcBlock
   * @param stream data stream to read from
   * @return BtcBlock
   */
  static BtcBlock fromVbkEncoding(ReadStream& stream);

  /**
   * Convert BtcBlock to data stream using BtcBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert BtcBlock to bytes data using BtcBlock basic byte format
   * @return string represantation of the data
   */
  std::vector<uint8_t> toRaw() const;

  /**
   * Convert BtcBlock to Hex string using BtcBlock basic byte format
   * @return Hex represantation of the data
   */
  std::string toHex() const;

  /**
   * Convert BtcBlock to data stream using BtcBlock VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  uint32_t getDifficulty() const { return _bits; }

  uint32_t getBlockTime() const { return _timestamp; }
  void setBlockTime(const uint32_t timestamp) { _timestamp = timestamp; }

  hash_t getPreviousBlock() const { return _previousBlock; }

  uint32_t getVersion() const { return _version; }

  uint256 getMerkleRoot() const { return _merkleRoot; }

  uint32_t getNonce() const { return _nonce; }
  void setNonce(const uint32_t nonce) { _nonce = nonce; }

  friend bool operator==(const BtcBlock& a, const BtcBlock& b) {
    // clang-format off
    return a._bits == b._bits &&
           a._version == b._version &&
           a._timestamp == b._timestamp &&
           a._nonce == b._nonce &&
           a._merkleRoot == b._merkleRoot &&
           a._previousBlock == b._previousBlock;
    // clang-format on
  }

  friend bool operator!=(const BtcBlock& a, const BtcBlock& b) {
    return !(a == b);
  }

  /**
   * Calculate the hash of the btc block
   * @return hash block hash
   */
  uint256 getHash() const;

  static std::string name() { return "BTC"; }

  std::string toPrettyString() const;

  uint32_t _version = 0;
  uint256 _previousBlock{};
  uint256 _merkleRoot{};
  uint32_t _timestamp = 0;
  uint32_t _bits = 0;
  uint32_t _nonce = 0;

 /*protected:
  uint32_t _version = 0;
  uint256 _previousBlock{};
  uint256 _merkleRoot{};
  uint32_t _timestamp = 0;
  uint32_t _bits = 0;
  uint32_t _nonce = 0;*/
};

template <typename JsonValue>
JsonValue ToJSON(const BtcBlock& b) {
  JsonValue object = json::makeEmptyObject<JsonValue>();
  json::putStringKV(object, "hash", HexStr(b.getHash()));
  json::putIntKV(object, "version", b.getVersion());
  json::putStringKV(object, "previousBlock", HexStr(b.getPreviousBlock()));
  json::putStringKV(object, "merkleRoot", HexStr(b.getMerkleRoot()));
  json::putIntKV(object, "timestamp", b.getBlockTime());
  json::putIntKV(object, "bits", b.getDifficulty());
  json::putIntKV(object, "nonce", b.getNonce());
  return object;
}

}  // namespace altintegration

namespace std {

template <>
struct hash<altintegration::BtcBlock> {
  size_t operator()(const altintegration::BtcBlock& block) {
    return altintegration::ArithUint256(block.getHash()).getLow64();
  }
};

}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
