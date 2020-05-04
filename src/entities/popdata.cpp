// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/popdata.hpp>

namespace altintegration {

PopData PopData::fromVbkEncoding(ReadStream& stream) {
  PopData alt_pop_tx;
  alt_pop_tx.version = stream.readBE<int32_t>();
  alt_pop_tx.vbk_context = readArrayOf<VbkBlock>(
      stream,
      0,
      MAX_CONTEXT_COUNT_ALT_PUBLICATION,
      (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding);

  alt_pop_tx.hasAtv = stream.readBE<uint8_t>();
  if (alt_pop_tx.hasAtv) {
    alt_pop_tx.atv = ATV::fromVbkEncoding(stream);
  }
  alt_pop_tx.vtbs = readArrayOf<VTB>(stream,
                                     0,
                                     MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                                     (VTB(*)(ReadStream&))VTB::fromVbkEncoding);

  return alt_pop_tx;
}

PopData PopData::fromVbkEncoding(Slice<const uint8_t> raw_bytes) {
  ReadStream stream(raw_bytes);
  return fromVbkEncoding(stream);
}

void PopData::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<int32_t>(version);
  writeSingleBEValue(stream, vbk_context.size());
  for (const auto& b : vbk_context) {
    b.toVbkEncoding(stream);
  }
  stream.writeBE<uint8_t>(hasAtv);
  if (hasAtv) {
    atv.toVbkEncoding(stream);
  }
  writeSingleBEValue(stream, vtbs.size());
  for (const auto& vtb : vtbs) {
    vtb.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> PopData::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

bool PopData::containsEndorsements() const { return hasAtv; }

}  // namespace altintegration
