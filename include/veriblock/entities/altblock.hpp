#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"

namespace altintegration {

struct AltBlock {
  using height_t = int32_t;
  using hash_t = std::vector<uint8_t>;

  hash_t hash{};
  uint32_t timestamp{};
  height_t height{};

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
   * Convert AltBlock to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltBlock to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  hash_t getHash() const { return hash; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
