// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/popdata.hpp>

#include "veriblock/hashutil.hpp"

namespace altintegration {

PopData PopData::fromVbkEncoding(ReadStream& stream) {
  PopData pd;
  pd.version = stream.readBE<uint32_t>();
  if (pd.version == 1) {
    pd.context = readArrayOf<VbkBlock>(
        stream,
        0,
        MAX_CONTEXT_COUNT,
        (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding);

    pd.atvs = readArrayOf<ATV>(stream,
                               0,
                               MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                               (ATV(*)(ReadStream&))ATV::fromVbkEncoding);

    pd.vtbs = readArrayOf<VTB>(stream,
                               0,
                               MAX_CONTEXT_COUNT_VBK_PUBLICATION,
                               (VTB(*)(ReadStream&))VTB::fromVbkEncoding);
  } else {
    throw std::domain_error(fmt::format(
        "PopData deserialization version={} is not implemented", pd.version));
  }
  return pd;
}

PopData PopData::fromVbkEncoding(Slice<const uint8_t> raw_bytes) {
  ReadStream stream(raw_bytes);
  return fromVbkEncoding(stream);
}

void PopData::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint32_t>(version);
  if (version == 1) {
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
  } else {
    VBK_ASSERT_MSG(
        false, "PopData serialization version=%d is not implemented", version);
  }
}

std::vector<uint8_t> PopData::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

bool Deserialize(ReadStream& stream, PopData& out, ValidationState& state) {
  PopData pd;
  typedef bool (*vbkde)(ReadStream&, VbkBlock&, ValidationState&);
  typedef bool (*atvde)(ReadStream&, ATV&, ValidationState&);
  typedef bool (*vtbde)(ReadStream&, VTB&, ValidationState&);

  if (stream.readBE<uint32_t>(pd.version, state)) {
    return state.Invalid("pop-version");
  }
  if (pd.version != 1) {
    return state.Invalid("pop-bad-version");
  }

  if (!readArrayOf<VbkBlock>(stream,
                             pd.context,
                             state,
                             0,
                             MAX_CONTEXT_COUNT,
                             static_cast<vbkde>(Deserialize))) {
    return state.Invalid("pop-vbk-context");
  }

  if (!readArrayOf<ATV>(stream,
                        pd.atvs,
                        state,
                        0,
                        MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                        static_cast<atvde>(Deserialize))) {
    return state.Invalid("pop-atv-context");
  }

  if (!readArrayOf<VTB>(stream,
                        pd.vtbs,
                        state,
                        0,
                        MAX_CONTEXT_COUNT_VBK_PUBLICATION,
                        static_cast<vtbde>(Deserialize))) {
    return state.Invalid("pop-vtb-context");
  }
  out = pd;
  return true;
}

}  // namespace altintegration
