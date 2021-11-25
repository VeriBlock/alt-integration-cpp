// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TREE_ALGO_HPP
#define ALTINTEGRATION_TREE_ALGO_HPP

#include <functional>
#include <stack>
#include <unordered_set>

#include "block_index.hpp"

namespace altintegration {

//! Postorder tree traversal algorithm.
template <typename Block>
void forEachNodePostorder(
    BlockIndex<Block>& index,
    const std::function<void(BlockIndex<Block>&)>& visit,
    const std::function<bool(BlockIndex<Block>&)>& shouldVisit) {
  using index_t = BlockIndex<Block>;
  std::unordered_set<index_t*> set;
  std::stack<index_t*> stack;
  stack.push(&index);
  while (!stack.empty()) {
    auto* item = stack.top();
    if (set.count(item) > 0) {
      visit(*item);
      stack.pop();
    } else {
      for (auto* next : item->pnext) {
        if (shouldVisit(*next)) {
          stack.push(next);
        }
      }
      set.insert(item);
    }
  }
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

/**
 * Find all tips after given block, including given block
 * @tparam Block
 */
template <typename Block>
std::vector<BlockIndex<Block>*> findValidTips(
    const std::unordered_set<BlockIndex<Block>*>& tips,
    BlockIndex<Block>& index) {
  using index_t = BlockIndex<Block>;
  std::vector<index_t*> ret{};
  for (auto* tip : tips) {
    auto* ancestor = tip->getAncestor(index.getHeight());
    if (ancestor == &index) {
      ret.push_back(tip);
    }
  }

  return ret;
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_TREE_ALGO_HPP
