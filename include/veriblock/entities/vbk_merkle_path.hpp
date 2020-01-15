#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_

#include <cstdint>

#include "veriblock/entities/hashes.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct VbkMerklePath {
  int32_t treeIndex{};
  int32_t index{};
  Sha256Hash subject{};
  std::vector<Sha256Hash> layers{};

  static VbkMerklePath fromRaw(ReadStream& stream) {
    VbkMerklePath mp;
    mp.treeIndex = readSingleBEValue<int32_t>(stream);
    mp.index = readSingleBEValue<int32_t>(stream);
    mp.subject =
        readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE);

    mp.layers = readArrayOf<Sha256Hash>(
        stream, 0, MAX_LAYER_COUNT_MERKLE, [](ReadStream& stream) {
          return readSingleByteLenValue(
              stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE);
        });

    return mp;
  }

  void toRaw(WriteStream& stream) const {
    writeSingleFixedBEValue<int32_t>(stream, treeIndex);
    writeSingleFixedBEValue<int32_t>(stream, index);
    writeSingleByteLenValue(stream, subject);
    writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());
    for (const auto& layer : layers) {
      writeSingleByteLenValue(stream, layer);
    }
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
