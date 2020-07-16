// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/blockchain/alt_block_addon.hpp"
#include "veriblock/entities/endorsements.hpp"
#include "veriblock/json.hpp"
#include "veriblock/serde.hpp"

namespace altintegration {

struct VbkBlock;

struct AltBlock {
  using height_t = int32_t;
  using hash_t = std::vector<uint8_t>;
  using prev_hash_t = std::vector<uint8_t>;
  using protecting_block_t = VbkBlock;
  using addon_t = AltBlockAddon;

  /**
   * Read basic blockheader data from the vector of bytes and convert it to
   * AltBlock
   * @param bytes data to read fromm
   * @return AltBlock
   */
  static AltBlock fromRaw(Slice<const uint8_t> bytes);

  /**
   * Read basic blockheader data from the stream and convert it to AltBlock
   * @param stream data stream to read from
   * @return AltBlock
   */
  static AltBlock fromRaw(ReadStream& stream);

  /**
   * Read VBK data from the stream and convert it to AltBlock
   * @param stream data stream to read from
   * @return AltBlock
   */
  static AltBlock fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * AltBlock
   * @param string data bytes to read from
   * @return AltBlock
   */
  static AltBlock fromVbkEncoding(const std::string& bytes);

  /**
   * Convert AltBlock to data stream using AltBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert AltBlock to bytes data using AltBlock basic byte format
   * @return string represantation of the data
   */
  std::vector<uint8_t> toRaw() const;

  /**
   * Convert AltBlock to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltBlock to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  uint32_t getBlockTime() const noexcept;

  hash_t getHash() const;

  friend bool operator==(const AltBlock& a, const AltBlock& b) {
    return a.hash == b.hash;
  }

  friend bool operator!=(const AltBlock& a, const AltBlock& b) {
    return !(a == b);
  }

  static std::string name() { return "ALT"; }

  std::string toPrettyString() const {
    return fmt::sprintf("AltBlock{height=%d, hash=%s}", height, HexStr(hash));
  }

  hash_t hash{};
  hash_t previousBlock{};
  uint32_t timestamp{};
  height_t height{};
};

template <typename JsonValue>
JsonValue ToJSON(const AltBlock& alt) {
  JsonValue object = json::makeEmptyObject<JsonValue>();
  json::putStringKV(object, "hash", HexStr(alt.getHash()));
  json::putStringKV(object, "previousBlock", HexStr(alt.previousBlock));
  json::putIntKV(object, "timestamp", alt.getBlockTime());
  json::putIntKV(object, "height", alt.height);
  return object;
}

/// custom gtest printer
inline void PrintTo(const AltBlock& block, ::std::ostream* os) {
  *os << block.toPrettyString();
}

}  // namespace altintegration

namespace std {

template <>
struct hash<std::vector<uint8_t>> {
  size_t operator()(const std::vector<uint8_t>& x) const {
    return std::hash<std::string>{}(std::string{x.begin(), x.end()});
  }
};

template <>
struct hash<altintegration::AltBlock> {
  size_t operator()(const altintegration::AltBlock& block) {
    std::hash<std::vector<uint8_t>> hasher;
    return hasher(block.getHash());
  }
};

}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
