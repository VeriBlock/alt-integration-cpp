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
#include <veriblock/storage/block_provider.hpp>
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

template <typename BlockTreeT>
bool SaveTree(
    BlockTreeT& tree,
    details::GenericBlockWriter<typename BlockTreeT::block_t>& writer,
    ValidationState& state) {
  for (auto& block : tree.getBlocks()) {
    auto& index = block.second;
    if (index->isDirty()) {
      index->unsetDirty();
      if (!writer.writeBlock(*index)) {
        return state.Invalid(
            "bad-provider",
            fmt::format("cannot write block into the block provider, block: %s",
                        index->toPrettyString()));
      }
    }
  }

  if (!writer.writeTip(*tree.getBestChain().tip())) {
    return state.Invalid(
        "bad-provider",
        fmt::format("cannot write tip into the block provider, tip: %s",
                    tree.getBestChain().tip()->toPrettyString()));
  }
  return true;
}

struct AltBlockTree;

bool SaveAllTrees(PopContext& context, ValidationState& state);

bool LoadAllTrees(PopContext& context, ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
