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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBK_MERKLE_PATH_HPP_
