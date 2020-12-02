// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_UTIL_HPP
#define ALT_INTEGRATION_UTIL_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/keystone_util.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

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

template <typename Block, typename ChainParams>
bool addBlocks(BlockTree<Block, ChainParams>& tree,
               const std::vector<std::vector<uint8_t>>& blocks,
               ValidationState& state) {
  for (const auto& b : blocks) {
    Block block = Block::fromRaw(b);
    if (!tree.acceptBlock(block, state)) {
      return false;
    }
  }

  return true;
}

//! Calculates ContextInfoContainerHash which cryptographically authenticates
//! block height, its position in blockchain (via keystones), and containing
//! PopData
uint256 CalculateContextInfoContainerHash(
    const PopData& popData,
    const BlockIndex<AltBlock>* prevBlock,
    const uint32_t keystoneInterval,
    const uint32_t altBootstrapHeight = 0);

//! @overload
uint256 CalculateContextInfoContainerHash(
    const PopData& popData,
    const BlockIndex<AltBlock>* prevBlock,
    const AltChainParams& params,
    const uint32_t altBootstrapHeight = 0);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_UTIL_HPP
