#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_

#include <cstdint>
#include <vector>
#include <cassert>
#include <stdexcept>

#include "veriblock/serde.hpp"
#include "veriblock/consts.hpp"

#include "veriblock/entities/hashes.hpp"

namespace VeriBlock {

struct MerklePath {
  std::vector<Sha256Hash> layers;
  Sha256Hash subject;
  int32_t index;

  MerklePath(int32_t _index,
             Sha256Hash _subject,
             std::vector<Sha256Hash> _layers)
      : index(_index), subject(_subject), layers(std::move(_layers)) {}

  static MerklePath fromRaw(ReadStream& stream, const Sha256Hash& subject) {
    int32_t index = readSingleBEValue<int32_t>(stream);
    const auto numLayers = readSingleBEValue<int32_t>(stream);
    checkRange(numLayers, 0, MAX_LAYER_COUNT_MERKLE);

    const auto sizeOfSizeBottomData = readSingleBEValue<int32_t>(stream);
    // TODO: if this assert failed, something is wrong with the code
    assert(sizeOfSizeBottomData == 4);
    // TODO: review this line
    const auto sizeOfBottomData = stream.readBE<int32_t>();
    if (sizeOfBottomData != SHA256_HASH_SIZE) {
      throw std::invalid_argument(
          "MerklePath.fromRaw(): bad size of bottom data");
    }

    std::vector<Sha256Hash> layers;
    layers.reserve(numLayers);
    for (int i = 0; i < numLayers; i++) {
      layers.emplace_back(
          readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE));
    }

    return MerklePath(index, subject, layers);
  }

  static MerklePath fromVbkEncoding(ReadStream& stream,
                                    const Sha256Hash& subject) {
    auto merkleBytes = readVarLenValue(stream, 0, MAX_MERKLE_BYTES);
    ReadStream merkleStream(merkleBytes);
    return MerklePath::fromRaw(merkleStream, subject);
  }

  void toRaw(WriteStream& stream) const {
    writeSingleFixedBEValue<int32_t>(stream, index);
    writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());

    const auto subjectSizeBytes = fixedArray((int32_t) subject.size());
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
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
