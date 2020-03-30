#include "veriblock/entities/vbk_merkle_path.hpp"

#include "veriblock/entities/vbkpoptx.hpp"

using namespace altintegration;

VbkMerklePath VbkMerklePath::fromVbkEncoding(ReadStream& stream) {
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

void VbkMerklePath::toVbkEncoding(WriteStream& stream) const {
  writeSingleFixedBEValue<int32_t>(stream, treeIndex);
  writeSingleFixedBEValue<int32_t>(stream, index);
  writeSingleByteLenValue(stream, subject);
  writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());
  for (const auto& layer : layers) {
    writeSingleByteLenValue(stream, layer);
  }
}

uint128 VbkMerklePath::calculateMerkleRoot() const {
  if (layers.empty()) {
    return subject.trim<VBK_MERKLE_ROOT_HASH_SIZE>();
  }

  uint256 cursor = subject;
  auto layerIndex = index;
  for (size_t i = 0, size = layers.size(); i < size; ++i) {
    auto& layer = layers[i];
    auto& left = layerIndex & 1u ? layer : cursor;
    auto& right = layerIndex & 1u ? cursor : layer;
    cursor = sha256(left, right);

    // Because a layer has processed but the index (i) hasn't progressed, these
    // values are offset by 1
    if (i == size - 2) {
      // metapackage hash is on the left
      layerIndex = 1;
    } else if (i == size - 3) {
      layerIndex = treeIndex;
    } else {
      layerIndex >>= 1u;
    }
  }

  return cursor.trim<VBK_MERKLE_ROOT_HASH_SIZE>();
}
