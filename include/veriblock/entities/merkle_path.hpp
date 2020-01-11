#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_

#include <cassert>
#include <stdexcept>

#include "veriblock/entities/hashes.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct MerklePath {
  std::vector<Sha256Hash> layers{};
  Sha256Hash subject{};
  int32_t index{};

  static MerklePath fromRaw(ReadStream& stream, const Sha256Hash& subject) {
    MerklePath path;
    path.subject = subject;
    path.index = readSingleBEValue<int32_t>(stream);
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

    path.layers.reserve(numLayers);
    for (int i = 0; i < numLayers; i++) {
      path.layers.emplace_back(
          readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE));
    }

    return path;
  }

  static MerklePath fromVbkEncoding(ReadStream& stream,
                                    const Sha256Hash& subject) {
    auto merkleBytes = readVarLenValue(stream, 0, MAX_MERKLE_BYTES);
    ReadStream merkleStream(merkleBytes);
    return MerklePath::fromRaw(merkleStream, subject);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
