// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/pop_context.hpp>
#include <veriblock/storage/block_reader.hpp>
#include <veriblock/validation_state.hpp>

#include "block_batch.hpp"

namespace altintegration {

//! efficiently loads `blocks` into tree (they must be sorted by height) and
//! does validation of these blocks. Sets tip after loading.
//! @invariant NOT atomic
template <typename BlockTreeT>
bool LoadBlocks(BlockTreeT& tree,
                std::vector<typename BlockTreeT::index_t>& blocks,
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

  // if tip of such chain exists in the tree we should not load it
  if (tree.getBlockIndex(blocks.back().getHash()) != nullptr) {
    return true;
  }

  for (auto& block : blocks) {
    // load blocks one by one
    if (!tree.loadBlock(block, state)) {
      return state.Invalid("load-tree");
    }
  }

  return tree.loadTip(tiphash, state);
}

template <typename BlockTreeT>
void SaveTree(BlockTreeT& tree, BlockBatch& batch) {
  using index_t = typename BlockTreeT::index_t;
  std::vector<const index_t*> dirty_indices;

  // map pair<hash, shared_ptr<index_t>> to vector<index_t*>
  for (auto& block : tree.getBlocks()) {
    auto& index = block.second;
    if (index->isDirty()) {
      index->unsetDirty();
      dirty_indices.push_back(index.get());
    }
  }

  // sort by height in descending order, because we need to calculate index hash
  // during saving. this is needed to be progpow-cache friendly, as cache will
  // be warm for last blocks.
  std::sort(dirty_indices.begin(),
            dirty_indices.end(),
            [](const index_t* a, const index_t* b) {
              return b->getHeight() < a->getHeight();
            });

  // write indices
  for (const index_t* index : dirty_indices) {
    batch.writeBlock(*index);
  }

  batch.writeTip(*tree.getBestChain().tip());
}

struct AltBlockTree;

void SaveAllTrees(const AltBlockTree& tree, BlockBatch& batch);

bool LoadAllTrees(PopContext& context,
                  BlockReader& reader,
                  ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
