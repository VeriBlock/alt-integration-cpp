// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/storage/block_batch_adaptor.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

//! efficiently loads `blocks` into tree (they must be sorted by height) and
//! does validation of these blocks. Sets tip after loading.
//! @invariant NOT atomic
template <typename BlockTreeT>
bool LoadTree(BlockTreeT& tree,
              std::vector<typename BlockTreeT::index_t> blocks,
              const typename BlockTreeT::hash_t& tiphash,
              ValidationState& state) {
  using index_t = typename BlockTreeT::index_t;
  using block_t = typename BlockTreeT::block_t;
  VBK_LOG_WARN("Loading %d %s blocks with tip %s",
               blocks.size(),
               block_t::name(),
               HexStr(tiphash));
  VBK_ASSERT(tree.isBootstrapped() && "tree must be bootstrapped");

  // first, sort them by height
  std::sort(
      blocks.begin(), blocks.end(), [](const index_t& a, const index_t& b) {
        return a.getHeight() < b.getHeight();
      });

  for (auto& block : blocks) {
    // load blocks one by one
    if (!tree.loadBlock(block, state)) {
      return state.Invalid("load-tree");
    }
  }

  return tree.loadTip(tiphash, state);
}

//! Save blocks and tip to batch
template <typename BlockTreeT>
void SaveTree(BlockTreeT& tree, BlockBatchAdaptor& batch) {
  for (auto& block : tree.getBlocks()) {
    auto& index = block.second;
    if (index->isDirty()) {
      index->unsetDirty();
      batch.writeBlock(*index);
    }
  }

  batch.writeTip(*tree.getBestChain().tip());
}

struct AltTree;

void SaveAllTrees(AltTree& tree, BlockBatchAdaptor& batch);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
