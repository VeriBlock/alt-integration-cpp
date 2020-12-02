// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/alt-util.hpp"

#include "veriblock/algorithm.hpp"
#include "veriblock/entities/merkle_tree.hpp"

namespace altintegration {

uint256 CalculateContextInfoContainerHash(const PopData& popData,
                                          const BlockIndex<AltBlock>* prevBlock,
                                          const uint32_t keystoneInterval,
                                          const uint32_t altBootstrapHeight) {
  WriteStream stream;
  if (prevBlock == nullptr) {
    // write height of ALTchain bootstrap block
    stream.writeBE<uint32_t>(altBootstrapHeight);
  } else {
    // write height of *this* block
    const auto height = prevBlock->getHeight() + 1;
    stream.writeBE<uint32_t>(height);

    const auto ki = keystoneInterval;
    const auto first = getFirstPreviousKeystoneHeight(height, ki);
    const auto second = getSecondPreviousKeystoneHeight(height, ki);

    // write first previous keystone
    auto firstPreviousKeystone = prevBlock->getAncestor(first);
    if (firstPreviousKeystone != nullptr) {
      stream.write(firstPreviousKeystone->getHash());

      // write second previous keystone
      auto secondPreviousKeystone = firstPreviousKeystone->getAncestor(second);
      if (secondPreviousKeystone != nullptr) {
        stream.write(secondPreviousKeystone->getHash());
      }
    }
  }

  auto popDataRoot = popData.getMerkleRoot();

  // calculate context hash
  auto contextHash = sha256twice(stream.data());

  return sha256twice(popDataRoot, contextHash);
}

uint256 CalculateContextInfoContainerHash(const PopData& popData,
                                          const BlockIndex<AltBlock>* prevBlock,
                                          const AltChainParams& params,
                                          const uint32_t altBootstrapHeight) {
  return CalculateContextInfoContainerHash(
      popData, prevBlock, params.getKeystoneInterval(), altBootstrapHeight);
}

}  // namespace altintegration