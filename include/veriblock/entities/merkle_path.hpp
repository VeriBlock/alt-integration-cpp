#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct MerklePath {
  int32_t index{};
  uint256 subject{};
  std::vector<uint256> layers{};

  static MerklePath fromRaw(ReadStream& stream, const uint256& subject) {
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

  static MerklePath fromVbkEncoding(ReadStream& stream,
                                    const uint256& subject) {
    auto merkleBytes = readVarLenValue(stream, 0, MAX_MERKLE_BYTES);
    ReadStream merkleStream(merkleBytes);
    return MerklePath::fromRaw(merkleStream, subject);
  }

  void toRaw(WriteStream& stream) const {
    writeSingleFixedBEValue<int32_t>(stream, index);
    writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());

    const auto subjectSizeBytes = fixedArray((int32_t)subject.size());
    writeSingleFixedBEValue<int32_t>(stream, (int32_t)subjectSizeBytes.size());
    stream.write(subjectSizeBytes);

    for (const auto& layer : layers) {
      writeSingleByteLenValue(stream, layer);
    }
  }

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream pathStream;
    toRaw(pathStream);
    writeVarLenValue(stream, pathStream.data());
  }

  uint256 calculateMerkleRoot() const {
    uint256 cursor = subject;
    int layerIndex = index;
    for (const auto& layer : layers) {
      if (layerIndex & 1) {
        std::vector<uint8_t> data(layer.begin(), layer.end());
        data.insert(data.end(), cursor.begin(), cursor.end());
        cursor = sha256twice(data);
      } else {
        std::vector<uint8_t> data(cursor.begin(), cursor.end());
        data.insert(data.end(), layer.begin(), layer.end());
        cursor = sha256twice(data);
      }

      layerIndex >>= 1;
    }

    return cursor;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
