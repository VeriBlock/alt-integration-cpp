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
  auto zeroHash = prevBlock.getHash();
  std::fill(zeroHash.begin(), zeroHash.end(), 0);

  auto height = prevBlock.getHeight() + 1;
  auto ki = params.getKeystoneInterval();

  auto firstPreviousKeystone =
      prevBlock.getAncestor(getFirstPreviousKeystoneHeight(height, ki));
  auto secondPreviousKeystone =
      firstPreviousKeystone != nullptr
          ? firstPreviousKeystone->getAncestor(
                getSecondPreviousKeystoneHeight(height, ki))
          : nullptr;

  WriteStream stream;
  stream.writeBE<uint32_t>(params.getKeystoneInterval());
  stream.write(firstPreviousKeystone != nullptr
                   ? firstPreviousKeystone->getHash()
                   : zeroHash);
  stream.write(secondPreviousKeystone != nullptr
                   ? secondPreviousKeystone->getHash()
                   : zeroHash);
  auto contextHash = sha256twice(stream.data());

  auto atvMerkleRoot =
      PayloadsMerkleTree<ATV>(map_get_id(popData.atvs)).getMerkleRoot();
  auto vtbMerkleRoot =
      PayloadsMerkleTree<VTB>(map_get_id(popData.vtbs)).getMerkleRoot();
  auto vbkMerkleRoot =
      PayloadsMerkleTree<VbkBlock>(map_get_id(popData.context)).getMerkleRoot();

  uint256 left = sha256twice(vbkMerkleRoot, vtbMerkleRoot);
  uint256 right = sha256twice(atvMerkleRoot, contextHash);

  return sha256twice(left, right);
}

}  // namespace altintegration