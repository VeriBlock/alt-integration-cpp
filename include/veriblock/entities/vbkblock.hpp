#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct VbkBlock {
  using hash_t = uint192;
  using height_t = int32_t;
  using keystone_t = uint72;

  int32_t height{};
  int16_t version{};
  uint96 previousBlock{};
  keystone_t previousKeystone{};
  keystone_t secondPreviousKeystone{};
  uint128 merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  int32_t nonce{};

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
    return a.height == b.height && a.version == b.version &&
           a.previousBlock == b.previousBlock &&
           a.previousKeystone == b.previousKeystone &&
           a.secondPreviousKeystone == b.secondPreviousKeystone &&
           a.merkleRoot == b.merkleRoot && a.timestamp == b.timestamp &&
           a.difficulty == b.difficulty && a.nonce == b.nonce;
  }

  friend bool operator!=(const VbkBlock& a, const VbkBlock& b) {
    return !(a == b);
  }

  /**
   * Calculate the hash of the vbk block
   * @return hash block hash
   */
  hash_t getHash() const;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
