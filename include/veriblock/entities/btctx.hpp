#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct BtcTx {
  std::vector<uint8_t> tx{};

  BtcTx() = default;

  BtcTx(const std::vector<uint8_t>& bytes) : tx(bytes) {}
  BtcTx(Slice<const uint8_t> slice) : tx(slice.begin(), slice.end()) {}

  /**
   * Read VBK data from the stream and convert it to BtcTx
   * @param stream data stream to read from
   * @return BtcTx
   */
  static BtcTx fromVbkEncoding(ReadStream& stream);

  /**
   * Convert BtcTx to data stream using BtcTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the btc transaction
   * @return hash transaction hash
   */
  uint256 getHash() const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
