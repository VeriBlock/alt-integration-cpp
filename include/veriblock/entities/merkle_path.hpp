// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct MerklePath {
  int32_t index{};
  uint256 subject{};
  std::vector<uint256> layers{};

  /**
   * Read basic data from the stream and convert it to MerklePath
   * @param stream data stream to read from
   * @return MerklePath
   */
  static MerklePath fromRaw(ReadStream& stream, const uint256& subject);

  /**
   * Read VBK data from the stream and convert it to MerklePath
   * @param stream data stream to read from
   * @return MerklePath
   */
  static MerklePath fromVbkEncoding(ReadStream& stream, const uint256& subject);

  /**
   * Convert MerklePath to data stream using MerklePath basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert MerklePath to data stream using MerklePath VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the merkle root
   * @return hash merkle root hash
   */
  uint256 calculateMerkleRoot() const;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
