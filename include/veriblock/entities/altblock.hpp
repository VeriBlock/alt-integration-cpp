// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include <veriblock/blockchain/alt_block_addon.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/hashers.hpp>
#include <veriblock/json.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

struct VbkBlock;

/**
 * @class AltBlock
 * Represents a view on Altchain block - i.e. contains partial info, only
 * required for POP.
 */
struct AltBlock {
  using height_t = int32_t;
  using hash_t = std::vector<uint8_t>;
  using prev_hash_t = std::vector<uint8_t>;
  using addon_t = AltBlockAddon;

  /**
   * Convert AltBlock to data stream using AltBlock basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  //! for AltBlock Raw format == VbkEncoding
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltBlock to bytes data using AltBlock basic byte format
   * @return string represantation of the data
   */
  std::vector<uint8_t> toRaw() const;

  size_t estimateSize() const;

  uint32_t getBlockTime() const noexcept;

  /**
   * @return hash of ALT block.
   */
  hash_t getHash() const;

  // dummy
  uint32_t getDifficulty() const { return 0; }

  friend bool operator==(const AltBlock& a, const AltBlock& b);
  friend bool operator!=(const AltBlock& a, const AltBlock& b);

  //! @private
  static const std::string& name() { return _name; }

  /**
   * Print this entity
   * @return
   */
  std::string toPrettyString() const;

  hash_t getPreviousBlock() const { return previousBlock; }
  uint32_t getTimestamp() const { return timestamp; }
  height_t getHeight() const { return height; }

  hash_t hash{};
  hash_t previousBlock{};
  uint32_t timestamp{};
  height_t height{};

 private:
  static const std::string _name;
};

template <typename JsonValue>
JsonValue ToJSON(const AltBlock& alt, bool reverseHashes = true) {
  JsonValue object = json::makeEmptyObject<JsonValue>();
  json::putStringKV(object, "hash", HexStr(alt.getHash(), reverseHashes));
  json::putStringKV(object, "previousBlock", HexStr(alt.previousBlock, reverseHashes));
  json::putIntKV(object, "timestamp", alt.getBlockTime());
  json::putIntKV(object, "height", alt.height);
  return object;
}

/// custom gtest printer
inline void PrintTo(const AltBlock& block, ::std::ostream* os) {
  *os << block.toPrettyString();
}

bool DeserializeFromRaw(
    ReadStream& stream,
    AltBlock& out,
    ValidationState& state,
    const AltBlock::hash_t& /* ignore */ = AltBlock::hash_t{});

bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    AltBlock& out,
    ValidationState& state,
    const AltBlock::hash_t& /* ignore */ = AltBlock::hash_t{});

}  // namespace altintegration

//! @private
template <>
struct std::hash<altintegration::AltBlock> {
  size_t operator()(const altintegration::AltBlock& block) {
    std::hash<std::vector<uint8_t>> hasher;
    return hasher(block.getHash());
  }
};

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
