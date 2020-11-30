// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/popdata.hpp>

#include "veriblock/hashutil.hpp"

namespace altintegration {

std::string PopData::toPrettyString() const {
  return fmt::sprintf("PopData{VBKs=%d, VTBs=%d, ATVs=%d, version=%d}",
                      context.size(),
                      vtbs.size(),
                      atvs.size(),
                      version);
}

// VbkBlocks, then VTBs, then ATVs
void PopData::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint32_t>(version);
  VBK_ASSERT_MSG(version == 1,
                 "PopData serialization version=%d is not implemented",
                 version);
  writeArrayOf<VbkBlock>(
      stream, context, [&](WriteStream& /*ignore*/, const VbkBlock& v) {
        v.toVbkEncoding(stream);
      });

  writeArrayOf<VTB>(stream, vtbs, [&](WriteStream& /*ignore*/, const VTB& v) {
    v.toVbkEncoding(stream);
  });

  writeArrayOf<ATV>(stream, atvs, [&](WriteStream& /*ignore*/, const ATV& atv) {
    atv.toVbkEncoding(stream);
  });
}

std::vector<uint8_t> PopData::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

size_t PopData::estimateSize() const {
  size_t size = 0;
  size += sizeof(version);
  if (version == 1) {
    size += singleBEValueSize(context.size());
    for (const auto& b : context) {
      size += b.estimateSize();
    }
    size += singleBEValueSize(atvs.size());
    for (const auto& atv : atvs) {
      size += atv.estimateSize();
    }
    size += singleBEValueSize(vtbs.size());
    for (const auto& vtb : vtbs) {
      size += vtb.estimateSize();
    }
  } else {
    VBK_ASSERT_MSG(
        false, "PopData estimate size version=%d is not implemented", version);
  }
  return size;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopData& pd,
                                ValidationState& state) {
  if (!stream.readBE<uint32_t>(pd.version, state)) {
    return state.Invalid("pop-version");
  }
  if (pd.version != 1) {
    return state.Invalid("pop-bad-version",
                         fmt::format("Expected version=1, got {}", pd.version));
  }

  size_t i = 0;
  if (!readArrayOf<VbkBlock>(
          stream, pd.context, state, 0, MAX_POPDATA_VBK, [&](VbkBlock& out) {
            i++;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-vbk", i);
  }

  i = 0;
  if (!readArrayOf<VTB>(
          stream, pd.vtbs, state, 0, MAX_POPDATA_VTB, [&](VTB& out) {
            i++;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-vtb", i);
  }

  i = 0;
  if (!readArrayOf<ATV>(
          stream, pd.atvs, state, 0, MAX_POPDATA_ATV, [&](ATV& out) {
            i++;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-atv", i);
  }

  return true;
}

}  // namespace altintegration
