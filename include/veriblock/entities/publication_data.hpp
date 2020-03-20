#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"

namespace altintegration {

struct PublicationData {
  int64_t identifier{};
  std::vector<uint8_t> header{};
  std::vector<uint8_t> payoutInfo{};
  std::vector<uint8_t> contextInfo{};

  /**
   * Read data from the vector of bytes and convert it to
   * PublicationData
   * @param stream data stream to read from
   * @return PublicationData
   */
  static PublicationData fromRaw(const std::vector<uint8_t> bytes);

  /**
   * Read data from the stream and convert it to PublicationData
   * @param stream data stream to read from
   * @return PublicationData
   */
  static PublicationData fromRaw(ReadStream& stream);

  /**
   * Convert PublicationData to data stream using PublicationData byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
