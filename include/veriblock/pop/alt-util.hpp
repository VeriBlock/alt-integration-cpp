// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_UTIL_HPP
#define ALT_INTEGRATION_UTIL_HPP

#include <algorithm>
#include <cstdint>
#include <vector>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/blocktree.hpp>
#include <veriblock/pop/entities/context_info_container.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "hashutil.hpp"
#include "keystone_util.hpp"

namespace altintegration {

//! Build a context (blocks necessary to connect) in a `tree` from `tip`
//! backwards `size` elements.
template <typename BlockTreeT>
std::vector<typename BlockTreeT::block_t> getContext(
    const BlockTreeT& tree,
    typename BlockTreeT::hash_t tip,
    size_t size = std::numeric_limits<size_t>::max()) {
  std::vector<typename BlockTreeT::block_t> ret;
  auto* cursor = tree.getBlockIndex(tip);
  size_t i = 0;
  while (cursor != nullptr && i++ < size) {
    ret.push_back(cursor->getHeader());
    cursor = cursor->pprev;
  }
  return ret;
}

//! helper to get last N known block hashes
template <typename BlockTree>
std::vector<std::vector<uint8_t>> getLastKnownBlocks(const BlockTree& tree,
                                                     size_t size) {
  std::vector<std::vector<uint8_t>> ret;
  ret.reserve(size);

  auto* tip = tree.getBestChain().tip();
  for (size_t i = 0; i < size && tip != nullptr; i++) {
    ret.push_back(tip->getHash().asVector());
    tip = tip->pprev;
  }

  // reverse them since we add them in reverse order (tip -> genesis)
  std::reverse(ret.begin(), ret.end());
  return ret;
}

//! helper to accept multiple blocks
template <typename Block, typename ChainParams>
bool addBlocks(BlockTree<Block, ChainParams>& tree,
               const std::vector<std::vector<uint8_t>>& blocks,
               ValidationState& state) {
  for (const auto& b : blocks) {
    Block block = Block::fromRaw(b);
    if (!tree.acceptBlockHeader(block, state)) {
      return false;
    }
  }

  return true;
}

/**
 * Creates a PublicationData - an entity, which stores information about
 * Altchain block endorsement.
 *
 * @param[in] endorsedBlockHeader endorsed block header
 * @param[in] txMerkleRoot original merkle root from altchain
 * @param[in] popData PopData from endorsed block
 * @param[in] payoutInfo payout info bytes.
 * @param[in] tree AltBlockTree instance
 * @param[out] out output instance
 * @return true if endorsed block found, false otherwise.
 */
bool GeneratePublicationData(const std::vector<uint8_t>& endorsedBlockHeader,
                             const std::vector<uint8_t>& txMerkleRoot,
                             const PopData& popData,
                             const std::vector<uint8_t>& payoutInfo,
                             const AltBlockTree& tree,
                             PublicationData& out);

//! @overload
PublicationData GeneratePublicationData(
    const std::vector<uint8_t>& endorsedBlockHeader,
    const BlockIndex<AltBlock>& endorsedBlock,
    const std::vector<uint8_t>& txMerkleRoot,
    const PopData& popData,
    const std::vector<uint8_t>& payoutInfo,
    const AltChainParams& params);

//! calculates top level merkle root that cryptographically authenticates
//! block content (transactions, PopData, context info) to a block
uint256 CalculateTopLevelMerkleRoot(const std::vector<uint8_t>& txMerkleRoot,
                                    const uint256& popDataMerkleRoot,
                                    const ContextInfoContainer& ctx);

//! @overload
uint256 CalculateTopLevelMerkleRoot(
    const AuthenticatedContextInfoContainer& ctx);

//! @overload
uint256 CalculateTopLevelMerkleRoot(const std::vector<uint8_t>& txMerkleRoot,
                                    const PopData& popData,
                                    const BlockIndex<AltBlock>* prevBlock,
                                    const AltChainParams& params);

//! The second to the last byte is used to determine the max number of occurrences
//! The first 7 bits determine the base (1 is added to the first 7 bits interpreted as a number),
//! and the last bit determines the exponent (0 = exponent of 1, 1 = exponent of 2)
//! For example:
//! 00000000 has a base of 1 (0+1) and an exponent of 1 for a maximum of 1 publication
//! 00000110 has a base of 4 (3+1) and an exponent of 1 for a maximum of 4 publications
//! 00000101 has a base of 3 (2+1) and an exponent of 2 for a maximum of 9 publications
int getMaxAtvsInVbkBlock(uint64_t altchainId);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_UTIL_HPP
