// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/merkle_path.hpp>

namespace altintegration {
void MerklePath::toRaw(WriteStream& stream) const {
  writeSingleFixedBEValue<int32_t>(stream, index);
  writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());

  const auto subjectSizeBytes = fixedArray((int32_t)subject.size());
  writeSingleFixedBEValue<int32_t>(stream, (int32_t)subjectSizeBytes.size());
  stream.write(subjectSizeBytes);

  for (const auto& layer : layers) {
    writeSingleByteLenValue(stream, layer);
  }
}

void MerklePath::toVbkEncoding(WriteStream& stream) const {
  WriteStream pathStream;
  toRaw(pathStream);
  writeVarLenValue(stream, pathStream.data());
}

size_t MerklePath::estimateSize() const {
  size_t rawSize = 0;
  rawSize += singleFixedBEValueSize(index);
  rawSize += singleFixedBEValueSize((int32_t)layers.size());

  const auto subjectSizeBytes = fixedArray((int32_t)subject.size());
  rawSize += singleFixedBEValueSize((int32_t)subjectSizeBytes.size());
  rawSize += subjectSizeBytes.size();

  for (const auto& layer : layers) {
    rawSize += singleByteLenValueSize(layer);
  }

  size_t size = 0;
  size += varLenValueSize(rawSize);

  return size;
}

uint256 MerklePath::calculateMerkleRoot() const {
  if (layers.empty()) {
    return subject;
  }

  auto cursor = subject;
  auto layerIndex = index;
  for (const auto& layer : layers) {
    auto& left = (layerIndex & 1u) != 0 ? layer : cursor;
    auto& right = (layerIndex & 1u) != 0 ? cursor : layer;
    cursor = sha256twice(left, right);
    layerIndex >>= 1u;
  }
  return cursor;
}

bool DeserializeFromRaw(ReadStream& stream,
                        const uint256& subject,
                        MerklePath& out,
                        ValidationState& state) {
  MerklePath path{};
  if (!readSingleBEValue<int32_t>(stream, path.index, state)) {
    return state.Invalid("merkle-index");
  }
  int32_t numLayers = 0;
  if (!readSingleBEValue<int32_t>(stream, numLayers, state)) {
    return state.Invalid("merkle-num-layers");
  }
  if (!checkRange(numLayers, 0, MAX_LAYER_COUNT_MERKLE, state)) {
    return state.Invalid("merkle-num-layers-range");
  }

  int32_t sizeOfSizeBottomData = 0;
  if (!readSingleBEValue<int32_t>(stream, sizeOfSizeBottomData, state)) {
    return state.Invalid("merkle-size-of-size");
  }
  if (sizeOfSizeBottomData != sizeof(int32_t)) {
    return state.Invalid("merkle-size-of-size-range");
  }

  int32_t sizeOfBottomData = 0;
  if (!stream.readBE<int32_t>(sizeOfBottomData, state)) {
    return state.Invalid("merkle-size-of-data");
  }
  if (sizeOfBottomData != SHA256_HASH_SIZE) {
    return state.Invalid("merkle-size-of-data-range");
  }

  path.layers.reserve(numLayers);
  for (int i = 0; i < numLayers; i++) {
    Slice<const uint8_t> layer;
    if (!readSingleByteLenValue(
            stream, layer, state, SHA256_HASH_SIZE, SHA256_HASH_SIZE)) {
      return state.Invalid("merkle-layer", i);
    }
    path.layers.emplace_back(layer);
  }

  path.subject = subject;
  out = path;
  return true;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                const uint256& subject,
                                MerklePath& out,
                                ValidationState& state) {
  Slice<const uint8_t> merkleBytes;
  if (!readVarLenValue(stream, merkleBytes, state, 0, MAX_POPDATA_SIZE)) {
    return state.Invalid("merkle-bytes");
  }
  ReadStream ms(merkleBytes);
  return DeserializeFromRaw(ms, subject, out, state);
}
}  // namespace altintegration