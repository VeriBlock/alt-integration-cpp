// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/merkle_path.hpp"

using namespace altintegration;

MerklePath MerklePath::fromRaw(ReadStream& stream, const uint256& subject) {
  MerklePath path{};
  path.index = readSingleBEValue<int32_t>(stream);
  const auto numLayers = readSingleBEValue<int32_t>(stream);
  checkRange(numLayers, 0, MAX_LAYER_COUNT_MERKLE);

  const auto sizeOfSizeBottomData = readSingleBEValue<int32_t>(stream);
  if (sizeOfSizeBottomData != sizeof(int32_t)) {
    throw std::invalid_argument(
        "MerklePath.fromRaw(): bad sizeOfSizeBottomData");
  }
  const auto sizeOfBottomData = stream.readBE<int32_t>();
  if (sizeOfBottomData != SHA256_HASH_SIZE) {
    throw std::invalid_argument(
        "MerklePath.fromRaw(): bad size of bottom data");
  }

  path.layers.reserve(numLayers);
  for (int i = 0; i < numLayers; i++) {
    path.layers.emplace_back(
        readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE));
  }

  path.subject = subject;
  return path;
}

MerklePath MerklePath::fromVbkEncoding(ReadStream& stream,
                                       const uint256& subject) {
  auto merkleBytes = readVarLenValue(stream, 0, MAX_MERKLE_BYTES);
  ReadStream merkleStream(merkleBytes);
  return MerklePath::fromRaw(merkleStream, subject);
}

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

uint256 MerklePath::calculateMerkleRoot() const {
  if (layers.empty()) {
    return subject;
  }

  auto cursor = subject;
  auto layerIndex = index;
  for (const auto& layer : layers) {
    auto& left = layerIndex & 1u ? layer : cursor;
    auto& right = layerIndex & 1u ? cursor : layer;
    cursor = sha256twice(left, right);
    layerIndex >>= 1u;
  }
  return cursor;
}

bool altintegration::DeserializeRaw(ReadStream& stream,
                                    const uint256& subject,
                                    MerklePath& out,
                                    ValidationState& state) {
  MerklePath path{};
  if (!readSingleBEValue<int32_t>(stream, path.index, state)) {
    return state.Invalid("invalid-index");
  }
  int32_t numLayers = 0;
  if (!readSingleBEValue<int32_t>(stream, numLayers, state)) {
    return state.Invalid("invalid-num-layers");
  }
  if (!checkRange(numLayers, 0, MAX_LAYER_COUNT_MERKLE, state)) {
    return state.Invalid("invalid-num-layers");
  }

  int32_t sizeOfSizeBottomData = 0;
  if (!readSingleBEValue<int32_t>(stream, sizeOfSizeBottomData, state)) {
    return state.Invalid("invalid-size-of-size");
  }
  if (sizeOfSizeBottomData != sizeof(int32_t)) {
    return state.Invalid("invalid-size-of-size");
  }

  int32_t sizeOfBottomData = 0;
  if (!stream.readBE<int32_t>(sizeOfBottomData, state)) {
    return state.Invalid("invalid-size-of-data");
  }
  if (sizeOfBottomData != SHA256_HASH_SIZE) {
    return state.Invalid("invalid-size-of-data");
  }

  path.layers.reserve(numLayers);
  for (int i = 0; i < numLayers; i++) {
    Slice<const uint8_t> layer;
    if (!readSingleByteLenValue(
            stream, layer, state, SHA256_HASH_SIZE, SHA256_HASH_SIZE)) {
      return state.Invalid("invalid-layer", i);
    }
    path.layers.emplace_back(layer);
  }

  path.subject = subject;
  out = path;
  return true;
}

bool altintegration::DeserializeRaw(Slice<const uint8_t> data,
                                    const uint256& subject,
                                    MerklePath& out,
                                    ValidationState& state) {
  ReadStream stream(data);
  return DeserializeRaw(stream, subject, out, state);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 const uint256& subject,
                                 MerklePath& out,
                                 ValidationState& state) {
  Slice<const uint8_t> merkleBytes;
  if (!readVarLenValue(stream, merkleBytes, state, 0, MAX_MERKLE_BYTES)) {
    return state.Invalid("invalid-merkle-bytes");
  }
  return DeserializeRaw(merkleBytes, subject, out, state);
}

bool altintegration::Deserialize(Slice<const uint8_t> data,
                                 const uint256& subject,
                                 MerklePath& out,
                                 ValidationState& state) {
  ReadStream stream(data);
  return Deserialize(stream, subject, out, state);
}
