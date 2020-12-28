// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/alt-util.hpp"
#include "veriblock/entities/keystone_container.hpp"
#include "veriblock/entities/publication_data.hpp"

namespace altintegration {

bool GeneratePublicationData(const std::vector<uint8_t>& endorsedBlockHeader,
                             const uint256& txMerkleRoot,
                             const PopData& popData,
                             const std::vector<uint8_t>& payoutInfo,
                             const AltBlockTree& tree,
                             PublicationData& out) {
  auto header_hash = tree.getParams().getHash(endorsedBlockHeader);
  auto index = tree.getBlockIndex(header_hash);

  if (index != nullptr) {
    out = GeneratePublicationData(endorsedBlockHeader,
                                  *index,
                                  txMerkleRoot,
                                  popData,
                                  payoutInfo,
                                  tree.getParams());
    return true;
  }
  return false;
}

PublicationData GeneratePublicationData(
    const std::vector<uint8_t>& endorsedBlockHeader,
    const BlockIndex<AltBlock>& endorsedBlock,
    const uint256& txMerkleRoot,
    const PopData& popData,
    const std::vector<uint8_t>& payoutInfo,
    const AltChainParams& params) {
  auto popDataRoot = popData.getMerkleRoot();
  auto ctx = AuthenticatedContextInfoContainer::createFromPrevious(
      sha256twice(txMerkleRoot, popDataRoot), endorsedBlock.pprev, params);

  PublicationData res;
  res.payoutInfo = payoutInfo;
  res.identifier = params.getIdentifier();
  res.contextInfo = SerializeToVbkEncoding(ctx);
  res.header = endorsedBlockHeader;

  return res;
}

uint256 CalculateTopLevelMerkleRoot(const uint256& txMerkleRoot,
                                    const PopData& popData,
                                    const BlockIndex<AltBlock>* prevBlock,
                                    const AltChainParams& params) {
  auto ctx = ContextInfoContainer::createFromPrevious(prevBlock, params);
  auto popDataMerkleRoot = popData.getMerkleRoot();
  return CalculateTopLevelMerkleRoot(txMerkleRoot, popDataMerkleRoot, ctx);
}

uint256 CalculateTopLevelMerkleRoot(
    const AuthenticatedContextInfoContainer& ctx) {
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