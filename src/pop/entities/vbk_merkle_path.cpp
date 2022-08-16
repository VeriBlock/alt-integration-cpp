// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <veriblock/pop/entities/vbk_merkle_path.hpp>

#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {
class WriteStream;
struct ReadStream;
}  // namespace altintegration

using namespace altintegration;

void VbkMerklePath::toVbkEncoding(WriteStream& stream) const {
  writeSingleFixedBEValue<int32_t>(stream, treeIndex);
  writeSingleFixedBEValue<int32_t>(stream, index);
  writeSingleByteLenValue(stream, subject);
  writeSingleFixedBEValue<int32_t>(stream, (int32_t)layers.size());
  for (const auto& layer : layers) {
    writeSingleByteLenValue(stream, layer);
  }
}

size_t VbkMerklePath::estimateSize() const {
  size_t size = 0;
  size += singleFixedBEValueSize(treeIndex);
  size += singleFixedBEValueSize(index);
  size += singleByteLenValueSize(subject);
  size += singleFixedBEValueSize((int32_t)layers.size());
  for (const auto& layer : layers) {
    size += singleByteLenValueSize(layer);
  }
  return size;
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
    auto& left = (layerIndex & 1u) != 0u ? layer : cursor;
    auto& right = (layerIndex & 1u) != 0u ? cursor : layer;
    cursor = sha256(left, right);
    layerIndex >>= 1u;
  }

  return cursor.trim<VBK_MERKLE_ROOT_HASH_SIZE>();
}

uint32_t VbkMerklePath::foo() const {
  uint32_t i = 0;

  uint256 cursor = subject;
  auto layerIndex = index;
  for (uint32_t size = layers.size(); i < size; ++i) {
    if (i == size - 1) {
      // metapackage hash is on the left
      layerIndex = 1;
    } else if (i == size - 2) {
      layerIndex = treeIndex;
    }

    auto& layer = layers[i];
    auto& left = (layerIndex & 1u) != 0u ? layer : cursor;
    auto& right = (layerIndex & 1u) != 0u ? cursor : layer;
    if (right == left) {
      return i;
    }
    cursor = sha256(left, right);
    layerIndex >>= 1u;
  }

  return i;
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                VbkMerklePath& path,
                                                ValidationState& state) {
  if (!readSingleBEValue<int32_t>(stream, path.treeIndex, state)) {
    return state.Invalid("vbkmerkle-tree-index");
  }
  if (!readSingleBEValue<int32_t>(stream, path.index, state)) {
    return state.Invalid("vbkmerkle-index");
  }
  if (!readSingleByteLenValue(
          stream, path.subject, state, SHA256_HASH_SIZE, SHA256_HASH_SIZE)) {
    return state.Invalid("vbkmerkle-subject");
  }

  if (!readArrayOf<uint256>(
          stream,
          path.layers,
          state,
          0,
          MAX_LAYER_COUNT_MERKLE,
          [](ReadStream& stream, uint256& out, ValidationState& state) {
            return readSingleByteLenValue(
                stream, out, state, SHA256_HASH_SIZE, SHA256_HASH_SIZE);
          })) {
    return state.Invalid("vbkmerkle-layers");
  }

  return true;
}
