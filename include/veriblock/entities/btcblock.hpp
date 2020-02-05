#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct BtcBlock {
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

  /**
   * Calculate the hash of the btc block
   * @return hash block hash
   */
  uint256 getHash() const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
