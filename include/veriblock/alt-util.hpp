#ifndef ALT_INTEGRATION_UTIL_HPP
#define ALT_INTEGRATION_UTIL_HPP

#include <algorithm>
#include <cstdint>
#include <vector>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/validation_state.hpp>

namespace AltIntegrationLib {

template <typename BlockTree>
std::vector<std::vector<uint8_t>> getLastKnownBlocks(const BlockTree& tree,
                                                     size_t size) {
  std::vector<std::vector<uint8_t>> ret;
  ret.reserve(size);

  auto* tip = tree.getBestChain().tip();
  while (tip != nullptr) {
    ret.push_back(tip->header.toRaw());
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

template <typename Block, typename ChainParams>
bool removeBlocks(BlockTree<Block, ChainParams>& tree,
                  const std::vector<std::vector<uint8_t>>& blocks) {
  for (const auto& b : blocks) {
    tree.invalidateBlockByHash(sha256twice(b));
  }

  return true;
}

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_UTIL_HPP
