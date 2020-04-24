#include "veriblock/entities/altpoptx.hpp"

namespace altintegration {

AltPopTx AltPopTx::fromVbkEncoding(ReadStream& stream) {
  AltPopTx alt_pop_tx;
  alt_pop_tx.version = stream.readBE<int32_t>();
  alt_pop_tx.atv = ATV::fromVbkEncoding(stream);
  alt_pop_tx.vtbs = readArrayOf<VTB>(
      stream,
      0,
      MAX_CONTEXT_COUNT_ALT_PUBLICATION,
      [](ReadStream& stream) -> VTB { return VTB::fromVbkEncoding(stream); });

  return alt_pop_tx;
}

AltPopTx AltPopTx::fromVbkEncoding(Slice<const uint8_t> raw_bytes) {
  ReadStream stream(raw_bytes);
  return fromVbkEncoding(raw_bytes);
}

void AltPopTx::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<int32_t>(version);
  atv.toVbkEncoding(stream);
  writeSingleBEValue(stream, vtbs.size());
  for (const auto& vtb : vtbs) {
    vtb.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> AltPopTx::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

}  // namespace altintegration
