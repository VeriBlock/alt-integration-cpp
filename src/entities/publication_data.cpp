#include "veriblock/entities/publication_data.hpp"

using namespace altintegration;

PublicationData PublicationData::fromRaw(const std::vector<uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

PublicationData PublicationData::fromRaw(ReadStream& stream) {
  PublicationData pub;
  pub.identifier = readSingleBEValue<int64_t>(stream);
  pub.header =
      readVarLenValue(stream, 0, MAX_HEADER_SIZE_PUBLICATION_DATA).asVector();
  pub.contextInfo =
      readVarLenValue(stream, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA).asVector();
  pub.payoutInfo =
      readVarLenValue(stream, 0, MAX_PAYOUT_SIZE_PUBLICATION_DATA).asVector();
  return pub;
}

void PublicationData::toRaw(WriteStream& stream) const {
  writeSingleBEValue(stream, identifier);
  writeVarLenValue(stream, header);
  writeVarLenValue(stream, contextInfo);
  writeVarLenValue(stream, payoutInfo);
}
