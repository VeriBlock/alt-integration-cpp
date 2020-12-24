// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/alt-util.hpp"

#include "veriblock/entities/keystone_container.hpp"

namespace altintegration {

uint256 CalculateTopLevelMerkleRoot(const uint256& txMerkleRoot,
                                    const PopData& popData,
                                    const BlockIndex<AltBlock>* prevBlock,
                                    const AltChainParams& params) {
  auto ctx = ContextInfoContainer::createFromPrevious(prevBlock, params);
  auto popDataMerkleRoot = popData.getMerkleRoot();
  return CalculateTopLevelMerkleRoot(txMerkleRoot, popDataMerkleRoot, ctx);
}

uint256 CalculateTopLevelMerkleRoot(const AuthenticatedContextInfoContainer& ctx) {
  return ctx.getTopLevelMerkleRoot();
}

uint256 CalculateTopLevelMerkleRoot(const uint256& txMerkleRoot,
                                    const uint256& popDataMerkleRoot,
                                    const ContextInfoContainer& ctx) {
  AuthenticatedContextInfoContainer c;
  c.stateRoot = sha256twice(txMerkleRoot, popDataMerkleRoot);
  c.ctx = ctx;
  return CalculateTopLevelMerkleRoot(c);
}

}  // namespace altintegration