#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/serde.hpp"

namespace altintegration {

struct ATV {
  VbkTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};

  /**
   * Read VBK data from the stream and convert it to ATV
   * @param stream data stream to read from
   * @return ATV
   */
  static ATV fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to ATV
   * @param string data bytes to read from
   * @return ATV
   */
  static ATV fromVbkEncoding(Slice<const uint8_t> bytes);

  /**
   * Convert ATV to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert ATV to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  static ATV fromHex(const std::string& h);

  friend bool operator==(const ATV& a, const ATV& b) {
    // clang-format off
    return a.toVbkEncoding() == b.toVbkEncoding();
    // clang-format on
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
