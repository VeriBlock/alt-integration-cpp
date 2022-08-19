// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/uint.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
struct ReadStream;

/**
 * @struct VbkMerklePath
 *
 * Path in Merkle tree, which proves that `subject` exists in the tree.
 */
struct VbkMerklePath {
  //! an indentifier of which transactions tree (pop=0, normal=1)
  int32_t treeIndex = 0;
  //! the index of the bottom data TxID in the block it came from
  int32_t index{};
  //! TxID that this merkle path authenticates
  uint256 subject{};
  //! the layers in the merkle path
  std::vector<uint256> layers{};

  /**
   * Convert VbkMerklePath to data stream using VbkMerklePath VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  /**
   * Calculate the hash of the vb merkle root
   * @return hash merkle root hash
   */
  uint128 calculateMerkleRoot() const;

  /**
   * Return layer indexes where left and right nodes are equal
   * @return indexes vector
   */
  std::vector<uint32_t> equalLayerIndexes() const;
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const VbkMerklePath& mp) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "treeIndex", mp.treeIndex);
  json::putIntKV(obj, "index", mp.index);
  json::putStringKV(obj, "subject", mp.subject.toHex());
  json::putArrayKV(obj, "layers", mp.layers);
  return obj;
}

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkMerklePath& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
