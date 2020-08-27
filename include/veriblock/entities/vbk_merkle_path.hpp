// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

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
   * Read VBK data from the stream and convert it to VbkMerklePath
   * @param stream data stream to read from
   * @return VbkMerklePath
   */
  static VbkMerklePath fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VbkMerklePath to data stream using VbkMerklePath VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the vb merkle root
   * @return hash merkle root hash
   */
  uint128 calculateMerkleRoot() const;
};

template <typename JsonValue>
JsonValue ToJSON(const VbkMerklePath& mp) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "treeIndex", mp.treeIndex);
  json::putIntKV(obj, "index", mp.index);
  json::putStringKV(obj, "subject", mp.subject.toHex());
  json::putArrayKV(obj, "layers", mp.layers);
  return obj;
}

bool Deserialize(ReadStream& stream,
                 VbkMerklePath& out,
                 ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
