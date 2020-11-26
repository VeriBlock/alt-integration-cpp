// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/algorithm.hpp"
#include "veriblock/alt-util.hpp"
#include "veriblock/entities/merkle_tree.hpp"

namespace altintegration {

uint256 CalculateContextInfoContainerHash(const PopData& popData,
                                          const BlockIndex<AltBlock>& prevBlock,
                                          uint32_t keystoneInterval) {
  auto firstPreviousKeystone =
      prevBlock.getAncestor(getFirstPreviousKeystoneHeight(
          prevBlock.getHeight() + 1, keystoneInterval));
  auto secondPreviousKeystone =
      firstPreviousKeystone->getAncestor(getSecondPreviousKeystoneHeight(
          prevBlock.getHeight() + 1, keystoneInterval));

  WriteStream stream;
  stream.writeBE<uint32_t>(keystoneInterval);
  stream.write(firstPreviousKeystone->getHash());
  stream.write(secondPreviousKeystone->getHash());

  PayloadsMerkleTree<ATV> atvMerkleTree(map_get_id(popData.atvs));
  PayloadsMerkleTree<VTB> vtbMerkleTree(map_get_id(popData.vtbs));
  PayloadsMerkleTree<VbkBlock> vbkMerkleTree(map_get_id(popData.context));

  return sha256twice(stream.data());
}

}  // namespace altintegration