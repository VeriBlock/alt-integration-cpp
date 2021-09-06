// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/entities/merkle_tree.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/hashutil.hpp>

namespace altintegration {

std::string PopData::toPrettyString() const {
  return format("PopData{{VBKs={}, VTBs={}, ATVs={}, version={}}}",
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
      stream, context, [](WriteStream& stream, const VbkBlock& v) {
        v.toVbkEncoding(stream);
      });

  writeArrayOf<VTB>(stream, vtbs, [](WriteStream& stream, const VTB& v) {
    v.toVbkEncoding(stream);
  });

  writeArrayOf<ATV>(stream, atvs, [](WriteStream& stream, const ATV& atv) {
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
  VBK_ASSERT_MSG(version == 1,
                 "PopData estimate size version=%d is not implemented",
                 version);
  size += estimateArraySizeOf<VbkBlock>(
      context, [](const VbkBlock& v) { return v.estimateSize(); });
  size += estimateArraySizeOf<VTB>(
      vtbs, [](const VTB& vtb) { return vtb.estimateSize(); });
  size += estimateArraySizeOf<ATV>(
      atvs, [](const ATV& atv) { return atv.estimateSize(); });
  return size;
}
uint256 PopData::getMerkleRoot() const {
  WriteStream stream;
  stream.writeBE<uint32_t>(version);

  auto vbytes = sha256twice(stream.data());
  auto atvMerkleRoot =
      PayloadsMerkleTree<ATV>(map_get_id(atvs)).getMerkleRoot();
  auto vtbMerkleRoot =
      PayloadsMerkleTree<VTB>(map_get_id(vtbs)).getMerkleRoot();
  auto vbkMerkleRoot =
      PayloadsMerkleTree<VbkBlock>(map_get_id(context)).getMerkleRoot();

  auto left = sha256twice(vbkMerkleRoot, vtbMerkleRoot);
  auto right = sha256twice(atvMerkleRoot, vbytes);
  return sha256twice(left, right);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopData& pd,
                                ValidationState& state) {
  if (!stream.readBE<uint32_t>(pd.version, state)) {
    return state.Invalid("pop-version");
  }
  if (pd.version != 1) {
    return state.Invalid("pop-bad-version",
                         format("Expected version=1, got {}", pd.version));
  }

  size_t i = 0;
  if (!readArrayOf<VbkBlock>(
          stream,
          pd.context,
          state,
          0,
          MAX_POPDATA_VBK,
          [&i](ReadStream& stream, VbkBlock& out, ValidationState& state) {
            ++i;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-vbk", i);
  }

  i = 0;
  if (!readArrayOf<VTB>(
          stream,
          pd.vtbs,
          state,
          0,
          MAX_POPDATA_VTB,
          [&i](ReadStream& stream, VTB& out, ValidationState& state) {
            ++i;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-vtb", i);
  }

  i = 0;
  if (!readArrayOf<ATV>(
          stream,
          pd.atvs,
          state,
          0,
          MAX_POPDATA_ATV,
          [&i](ReadStream& stream, ATV& out, ValidationState& state) {
            ++i;
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("popdata-atv", i);
  }

  return true;
}

}  // namespace altintegration
