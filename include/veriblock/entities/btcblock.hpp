#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct BtcBlock {
  using hash_t = uint256;
  using height_t = uint32_t;

  uint32_t version{};
  uint256 previousBlock{};
  uint256 merkleRoot{};
  uint32_t timestamp{};
  uint32_t bits{};
  uint32_t nonce{};

  /**
   * Read basic blockheader data from the vector of bytes and convert it to
   * BtcBlock
   * @param stream data stream to read from
   * @return BtcBlock
   */
  static BtcBlock fromRaw(const std::vector<uint8_t>& bytes);

  /**
   * Read basic blockheader data from the stream and convert it to BtcBlock
   * @param stream data stream to read from
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
   * Convert BtcBlock to data stream using BtcBlock VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  uint32_t getDifficulty() const { return bits; }

  uint32_t getBlockTime() const { return timestamp; }

  ArithUint256 getBlockProof() const {
    bool negative;
    bool overflow;
    ArithUint256 bnTarget = ArithUint256::fromBits(bits, &negative, &overflow);
    if (negative || overflow || bnTarget == 0) {
      return 0;
    }

    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for an arith_uint256. However, as 2**256 is at least as
    // large as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) /
    // (bnTarget+1)) + 1, or ~bnTarget / (bnTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
  }

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

  /**
   * Calculate the hash of the btc block
   * @return hash block hash
   */
  uint256 getHash() const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
