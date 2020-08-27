// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/popdata.hpp>

#include "veriblock/hashutil.hpp"

namespace altintegration {

PopData PopData::fromVbkEncoding(ReadStream& stream) {
  PopData alt_pop_tx;
  alt_pop_tx.context = readArrayOf<VbkBlock>(
      stream,
      0,
      MAX_CONTEXT_COUNT_ALT_PUBLICATION,
      (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding);

  alt_pop_tx.atvs = readArrayOf<ATV>(stream,
                                     0,
                                     MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                                     (ATV(*)(ReadStream&))ATV::fromVbkEncoding);

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
  writeSingleBEValue(stream, context.size());
  for (const auto& b : context) {
    b.toVbkEncoding(stream);
  }

  writeSingleBEValue(stream, atvs.size());
  for (const auto& atv : atvs) {
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

PopData::id_t PopData::getHash() const {
  auto bytes = toVbkEncoding();
  return sha256(bytes);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 PopData& out,
                                 ValidationState& state) {
  PopData pd;
  typedef bool (*vbkde)(ReadStream&, VbkBlock&, ValidationState&);
  typedef bool (*atvde)(ReadStream&, ATV&, ValidationState&);
  typedef bool (*vtbde)(ReadStream&, VTB&, ValidationState&);

  if (!readArrayOf<VbkBlock>(stream,
                             pd.context,
                             state,
                             0,
                             MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                             static_cast<vbkde>(Deserialize))) {
    return state.Invalid("bad-vbk-context");
  }

  if (!readArrayOf<ATV>(stream,
                        pd.atvs,
                        state,
                        0,
                        MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                        static_cast<atvde>(Deserialize))) {
    return state.Invalid("bad-atv-context");
  }

  if (!readArrayOf<VTB>(stream,
                        pd.vtbs,
                        state,
                        0,
                        MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                        static_cast<vtbde>(Deserialize))) {
    return state.Invalid("bad-vtb-context");
  }
  out = pd;
  return true;
}

}  // namespace altintegration
