#include "veriblock/entities/payloads.hpp"

using namespace altintegration;

AltProof AltProof::fromVbkEncoding(ReadStream& stream) {
  AltProof altProof{};
  altProof.endorsed = AltBlock::fromVbkEncoding(stream);
  altProof.containing = AltBlock::fromVbkEncoding(stream);
  altProof.hasAtv = readSingleBEValue<uint8_t>(stream);
  if (altProof.hasAtv) {
    altProof.atv = ATV::fromVbkEncoding(stream);
  }

  return altProof;
}

AltProof AltProof::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void AltProof::toVbkEncoding(WriteStream& stream) const {
  endorsed.toVbkEncoding(stream);
  containing.toVbkEncoding(stream);
  writeSingleBEValue(stream, (uint8_t)hasAtv);
  atv.toVbkEncoding(stream);
}

std::vector<uint8_t> AltProof::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

Payloads Payloads::fromVbkEncoding(ReadStream& stream) {
  Payloads p;
  p.alt = AltProof::fromVbkEncoding(stream);
  p.vtbs =
      readArrayOf<VTB>(stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return VTB::fromVbkEncoding(stream);
      });

  p.btccontext = readArrayOf<BtcBlock>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return BtcBlock::fromVbkEncoding(stream);
      });

  p.vbkcontext = readArrayOf<VbkBlock>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return VbkBlock::fromVbkEncoding(stream);
      });

  return p;
}

Payloads Payloads::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void Payloads::toVbkEncoding(WriteStream& stream) const {
  alt.toVbkEncoding(stream);

  writeSingleBEValue(stream, vtbs.size());
  for (const auto& el : vtbs) {
    el.toVbkEncoding(stream);
  }

  writeSingleBEValue(stream, btccontext.size());
  for (const auto& el : btccontext) {
    el.toVbkEncoding(stream);
  }

  writeSingleBEValue(stream, vbkcontext.size());
  for (const auto& el : vbkcontext) {
    el.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> Payloads::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}
