// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

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
    if (i == size - 1) {
      // metapackage hash is on the left
      layerIndex = 1;
    } else if (i == size - 2) {
      layerIndex = treeIndex;
    }

    auto& layer = layers[i];
    auto& left = layerIndex & 1u ? layer : cursor;
    auto& right = layerIndex & 1u ? cursor : layer;
    cursor = sha256(left, right);
    layerIndex >>= 1u;
  }

  return cursor.trim<VBK_MERKLE_ROOT_HASH_SIZE>();
}
