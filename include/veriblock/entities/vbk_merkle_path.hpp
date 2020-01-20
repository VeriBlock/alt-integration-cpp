#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/consts.hpp"

#include "veriblock/entities/hashes.hpp"

namespace VeriBlock {

struct VbkMerklePath {
  int32_t treeIndex{};
  int32_t index{};
  Sha256Hash subject{};
  std::vector<Sha256Hash> layers{};

  static VbkMerklePath fromVbkEncoding(ReadStream& stream) {
    VbkMerklePath path{};
    path.treeIndex = readSingleBEValue<int32_t>(stream);
    path.index = readSingleBEValue<int32_t>(stream);
    path.subject =
        readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE);

    path.layers = readArrayOf<Sha256Hash>(
        stream, 0, MAX_LAYER_COUNT_MERKLE, [](ReadStream& stream) {
          return readSingleByteLenValue(
              stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE);
        });

    return path;
  }

  void toVbkEncoding(WriteStream& stream) const {
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
