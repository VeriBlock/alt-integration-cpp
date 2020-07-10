// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/logger.hpp>
#include <veriblock/storage/batch.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

//! efficiently loads `blocks` into tree (they must be sorted by height) and
//! does validation of these blocks. Sets tip after loading.
//! @invariant NOT atomic
template <typename BlockTreeT>
bool LoadTree(
    BlockTreeT& tree,
    const std::vector<std::pair<int, typename BlockTreeT::index_t>>& blocks,
    const typename BlockTreeT::hash_t& tiphash,
    ValidationState& state) {
  VBK_LOG_WARN("Loading %d blocks with tip %s", blocks.size(), HexStr(tiphash));

  for (auto& block : blocks) {
    // load blocks one by one
    if (!tree.loadBlock(block.second, state)) {
      return state.Invalid("load-blocks");
    }
  }

  return tree.loadTip(tiphash, state);
}

//! Save blocks and tip to batch
template <typename BlockTreeT>
void SaveTree(const BlockTreeT& tree, Batch& batch) {
  // TODO: add dirty flag and write only dirty blocks
  for (auto& block : tree.getBlocks()) {
    batch.writeBlock(*block.second);
  }

  batch.writeTip(*tree.getBestChain().tip());
}


struct AltTree;

void SaveAllTrees(const AltTree& tree, Batch& batch);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
