// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/storage/batch_adaptor.hpp>
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
  VBK_ASSERT(tree.isBootstrapped() && "tree must be bootstrapped");

  for (auto& block : blocks) {
    // load blocks one by one
    typename BlockTreeT::index_t blockTmp = block.second;
    blockTmp.status = BLOCK_VALID_TREE;
    if (block.second.hasFlags(BLOCK_BOOTSTRAP))
      blockTmp.status |= BLOCK_BOOTSTRAP;
    if (!tree.loadBlock(blockTmp, state)) {
      return state.Invalid("load-tree");
    }
  }

  return tree.loadTip(tiphash, state);
}

//! Save blocks and tip to batch
template <typename BlockTreeT>
void SaveTree(const BlockTreeT& tree, BatchAdaptor& batch) {
  // TODO: add dirty flag and write only dirty blocks
  for (auto& block : tree.getBlocks()) {
    batch.writeBlock(*block.second);
    block.second->unsetDirty();
  }

  batch.writeTip(*tree.getBestChain().tip());
}

struct AltTree;

void SaveAllTrees(const AltTree& tree, BatchAdaptor& batch);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
