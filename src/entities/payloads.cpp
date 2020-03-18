#include "veriblock/entities/payloads.hpp"

using namespace AltIntegrationLib;

AltProof AltProof::fromVbkEncoding(ReadStream& stream) {
  AltProof altProof{};
  altProof.endorsed = AltBlock::fromVbkEncoding(stream);
  altProof.containing = AltBlock::fromVbkEncoding(stream);
  altProof.atv = ATV::fromVbkEncoding(stream);

  return altProof;
}

AltProof AltProof::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void AltProof::toVbkEncoding(WriteStream& stream) const {
  endorsed.toVbkEncoding(stream);
  containing.toVbkEncoding(stream);
  atv.toVbkEncoding(stream);
}

std::vector<uint8_t> AltProof::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}
