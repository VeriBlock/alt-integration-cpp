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
                                          const AltChainParams& params) {
  auto firstPreviousKeystone =
      prevBlock.getAncestor(getFirstPreviousKeystoneHeight(
          prevBlock.getHeight() + 1, params.getKeystoneInterval()));
  auto secondPreviousKeystone =
      firstPreviousKeystone->getAncestor(getSecondPreviousKeystoneHeight(
          prevBlock.getHeight() + 1, params.getKeystoneInterval()));

  WriteStream stream;
  stream.writeBE<uint32_t>(params.getKeystoneInterval());
  stream.write(firstPreviousKeystone->getHash());
  stream.write(secondPreviousKeystone->getHash());

  auto atvMerkleRoot =
      PayloadsMerkleTree<ATV>(map_get_id(popData.atvs)).getMerkleRoot();
  auto vtbMerkleRoot =
      PayloadsMerkleTree<VTB>(map_get_id(popData.vtbs)).getMerkleRoot();
  auto vbkMerkleRoot =
      PayloadsMerkleTree<VbkBlock>(map_get_id(popData.context)).getMerkleRoot();

  uint256 left = sha256twice(atvMerkleRoot, vtbMerkleRoot);
  uint256 right = sha256twice(vbkMerkleRoot, vbkMerkleRoot);

  right = sha256twice(left, right);
  left = sha256twice(stream.data());

  return sha256twice(left, right);
}

}  // namespace altintegration