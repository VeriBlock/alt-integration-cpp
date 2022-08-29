// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/entities/publication_data.hpp>

#include "veriblock/pop/algorithm.hpp"
#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/blockchain/alt_block_tree.hpp"
#include "veriblock/pop/blockchain/alt_chain_params.hpp"
#include "veriblock/pop/entities/context_info_container.hpp"
#include "veriblock/pop/entities/popdata.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/serde.hpp"

namespace altintegration {
struct AltBlock;

bool GeneratePublicationData(const std::vector<uint8_t>& endorsedBlockHeader,
                             const std::vector<uint8_t>& txMerkleRoot,
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
    const std::vector<uint8_t>& txMerkleRoot,
    const PopData& popData,
    const std::vector<uint8_t>& payoutInfo,
    const AltChainParams& params) {
  auto atv_ids = map_get_id(popData.atvs);
  auto vtb_ids = map_get_id(popData.vtbs);
  auto vbk_ids = map_get_id(popData.context);
  return GeneratePublicationData(endorsedBlockHeader,
                                 endorsedBlock,
                                 txMerkleRoot,
                                 popData.version,
                                 atv_ids,
                                 vtb_ids,
                                 vbk_ids,
                                 payoutInfo,
                                 params);
}

PublicationData GeneratePublicationData(
    const std::vector<uint8_t>& endorsedBlockHeader,
    const BlockIndex<AltBlock>& endorsedBlock,
    const std::vector<uint8_t>& txMerkleRoot,
    uint32_t version,
    const std::vector<ATV::id_t>& atv_ids,
    const std::vector<VTB::id_t>& vtb_ids,
    const std::vector<VbkBlock::id_t>& vbk_ids,
    const std::vector<uint8_t>& payoutInfo,
    const AltChainParams& params) {
  auto popDataRoot = PopData::getMerkleRoot(version, atv_ids, vtb_ids, vbk_ids);
  auto ctx = AuthenticatedContextInfoContainer::createFromPrevious(
      sha256twice(txMerkleRoot, popDataRoot), endorsedBlock.pprev, params);

  PublicationData res;
  res.payoutInfo = payoutInfo;
  res.identifier = params.getIdentifier();
  res.contextInfo = SerializeToVbkEncoding(ctx);
  res.header = endorsedBlockHeader;

  return res;
}

uint256 CalculateTopLevelMerkleRoot(const std::vector<uint8_t>& txMerkleRoot,
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

uint256 CalculateTopLevelMerkleRoot(const std::vector<uint8_t>& txMerkleRoot,
                                    const uint256& popDataMerkleRoot,
                                    const ContextInfoContainer& ctx) {
  AuthenticatedContextInfoContainer c;
  c.stateRoot = sha256twice(txMerkleRoot, popDataMerkleRoot);
  c.ctx = ctx;
  return CalculateTopLevelMerkleRoot(c);
}

int getMaxAtvsInVbkBlock(uint64_t altchainId) {
  uint8_t last = altchainId & 0xFFu;
  if (last != 0xff) {
    return std::numeric_limits<int>::max();
  }

  uint8_t amountByte = (altchainId >> 8) & 0xFFu;
  int base = (((int)amountByte & 0x000000FFu) >> 1) + 1;
  uint8_t exponent = (amountByte & 0x01u) + 1;

  if (exponent == 1) {
    return base;
  }

  return base * base;
}

}  // namespace altintegration
