// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TREE_ALGO_HPP
#define ALTINTEGRATION_TREE_ALGO_HPP

#include <deque>
#include <functional>
#include <veriblock/blockchain/block_index.hpp>

namespace altintegration {

template <typename Block>
void forEachNodePostorder(
    BlockIndex<Block>& index,
    const std::function<void(BlockIndex<Block>&)>& visit) {
  // because pnext can be modified while iterating, we make a copy and iterate
  // over a copy
  auto copy = index.pnext;
  for (auto* pnext : copy) {
    VBK_ASSERT(pnext != nullptr);
    forEachNodePostorder(*pnext, visit);
  }

  visit(index);
}

//! iterate across all subtrees starting (and including) given 'index'
template <typename Block>
void forEachNodePreorder(BlockIndex<Block>& index,
                         const std::function<bool(BlockIndex<Block>&)>& visit) {
  if (!visit(index)) {
    // we should not continue traversal of this subtree
    return;
  }

  // because pnext can be modified while iterating, we make a copy and iterate
  // over a copy
  auto copy = index.pnext;
  for (auto* pnext : copy) {
    VBK_ASSERT(pnext != nullptr);
    forEachNodePreorder(*pnext, visit);
  }
}

//! iterate across all subtrees starting (and excluding) given 'index'
template <typename Block>
void forEachNextNodePreorder(
    BlockIndex<Block>& index,
    const std::function<bool(BlockIndex<Block>&)>& shouldContinue) {
  for (auto* pnext : index.pnext) {
    VBK_ASSERT(pnext != nullptr);
    if (shouldContinue(*pnext)) {
      forEachNextNodePreorder(*pnext, shouldContinue);
    }
  }
}

/**
 * Find all tips after given block, including given block
 * @tparam Block
 */
template <typename Block>
std::vector<BlockIndex<Block>*> findValidTips(BlockIndex<Block>& index) {
  using index_t = BlockIndex<Block>;
  std::vector<index_t*> ret{};
  forEachNodePreorder<Block>(index, [&ret](index_t& next) -> bool {
    if (!next.isValid()) {
      // this is invalid subtree
      return false;
    }

    // this is valid subtree
    if (next.isValidTip()) {
      ret.push_back(&next);
      // do not continue, as there are no valid next blocks
      return false;
    }

    return true;
  });

  return ret;
}
}  // namespace altintegration

#endif  // ALTINTEGRATION_TREE_ALGO_HPP
