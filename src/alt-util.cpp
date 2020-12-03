// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/alt-util.hpp"

#include "veriblock/entities/keystone_container.hpp"

namespace altintegration {

uint256 CalculateContextInfoContainerHash(const PopData& popData,
                                          const BlockIndex<AltBlock>* prevBlock,
                                          const uint32_t keystoneInterval,
                                          const uint32_t altBootstrapHeight) {
  WriteStream stream;
  const uint32_t height =
      prevBlock == nullptr ? altBootstrapHeight : prevBlock->getHeight() + 1u;
  // first, write 4 BE bytes - block height
  stream.writeBE<uint32_t>(height);

  // find and write previous keystones.
  // if keystone does not exist, do not write anything.
  // otherwise, write its hash
  KeystoneContainer::fromPrevious(prevBlock, keystoneInterval).write(stream);

  // calculate PopData sha256double merkle root which includes 'version'
  auto popDataRoot = popData.getMerkleRoot();

  // calculate unauthenticated ContextInfoContainer hash
  auto unauthContextInfoHash = sha256twice(stream.data());

  // calculate authenticated ContextInfoContainer hash
  return sha256twice(popDataRoot, unauthContextInfoHash);
}

uint256 CalculateContextInfoContainerHash(const PopData& popData,
                                          const BlockIndex<AltBlock>* prevBlock,
                                          const AltChainParams& params) {
  return CalculateContextInfoContainerHash(
      popData,
      prevBlock,
      params.getKeystoneInterval(),
      params.getBootstrapBlock().getHeight());
}

}  // namespace altintegration