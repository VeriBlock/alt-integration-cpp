#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct VbkBlock {
  using hash_t = uint192;
  using height_t = int32_t;

  int32_t height{};
  int16_t version{};
  uint144 previousBlock{};
  uint72 previousKeystone{};
  uint72 secondPreviousKeystone{};
  uint128 merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  int32_t nonce{};

  /**
   * Read basic blockheader data from the vector of bytes and convert it to
   * VbkBlock
   * @param stream data stream to read from
   * @return VbkBlock
   */
  static VbkBlock fromRaw(const std::vector<uint8_t>& bytes);

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
   * Convert VbkBlock to data stream using VbkBlock VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /*
   * TODO
   */
  uint32_t getDifficulty() const;

  /*
   * TODO
   */
  uint32_t getBlockTime() const;

  /*
   * TODO
   */
  ArithUint256 getBlockProof() const;

  friend bool operator==(const VbkBlock& a, const VbkBlock& b) {
    return a.height == b.height && a.version == b.version &&
           a.previousBlock == b.previousBlock &&
           a.previousKeystone == b.previousKeystone &&
           a.secondPreviousKeystone == b.secondPreviousKeystone &&
           a.merkleRoot == b.merkleRoot && a.timestamp == b.timestamp &&
           a.difficulty == b.difficulty && a.nonce == b.nonce;
  }

  /**
   * Calculate the hash of the vbk block
   * @return hash block hash
   */
  uint192 getHash() const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
