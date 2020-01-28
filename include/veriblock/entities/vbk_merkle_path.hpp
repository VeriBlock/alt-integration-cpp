#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct VbkMerklePath {
  int32_t treeIndex{};
  int32_t index{};
  uint256 subject{};
  std::vector<uint256> layers{};

  static VbkMerklePath fromVbkEncoding(ReadStream& stream) {
    VbkMerklePath path{};
    path.treeIndex = readSingleBEValue<int32_t>(stream);
    path.index = readSingleBEValue<int32_t>(stream);
    path.subject =
        readSingleByteLenValue(stream, SHA256_HASH_SIZE, SHA256_HASH_SIZE);

    path.layers = readArrayOf<uint256>(
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

  uint128 calculateMerkleRoot() const {
    uint256 cursor = subject;
    int layerIndex = index;
    for (size_t i = 0; i < layers.size(); ++i) {
      if (i == layers.size() - 1) {
        /* The last layer is the BlockContentMetapackage hash and will always be
           the "left" side, so set the layerIndex to 1 */
        layerIndex = 1;
      } else if (i == layers.size() - 2) {
        /* The second to last layer is the joining with the opposite transaction
           type group (normal vs pop), so use the tree index specified in the
           compact format */
        layerIndex = treeIndex;
      }
      if (layerIndex & 1) {
        std::vector<uint8_t> data(layers[i].begin(), layers[i].end());
        data.insert(data.end(), cursor.begin(), cursor.end());
        cursor = sha256(data);
      } else {
        std::vector<uint8_t> data(cursor.begin(), cursor.end());
        data.insert(data.end(), layers[i].begin(), layers[i].end());
        cursor = sha256(data);
      }

      layerIndex >>= 1;
    }

    return trim<VBK_MERKLE_ROOT_HASH_SIZE, SHA256_HASH_SIZE>(cursor);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
