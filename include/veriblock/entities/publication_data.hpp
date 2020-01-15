#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"

namespace VeriBlock {

struct PublicationData {
  int64_t identifier{};
  std::vector<uint8_t> header{};
  std::vector<uint8_t> payoutInfo{};
  std::vector<uint8_t> contextInfo{};

  static PublicationData fromRaw(ReadStream& stream) {
    PublicationData pub;
    pub.identifier = readSingleBEValue<int64_t>(stream);
    pub.header =
        readVarLenValue(stream, 0, MAX_HEADER_SIZE_PUBLICATION_DATA).asVector();
    pub.contextInfo =
        readVarLenValue(stream, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA)
            .asVector();
    pub.payoutInfo =
        readVarLenValue(stream, 0, MAX_PAYOUT_SIZE_PUBLICATION_DATA).asVector();
    return pub;
  }

  void toRaw(WriteStream& stream) const {
    writeSingleBEValue(stream, identifier);
    writeVarLenValue(stream, header);
    writeVarLenValue(stream, contextInfo);
    writeVarLenValue(stream, payoutInfo);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
