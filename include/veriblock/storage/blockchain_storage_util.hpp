// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKCHAIN_STORAGE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKCHAIN_STORAGE_UTIL_HPP_

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/storage/pop_storage.hpp>

namespace altintegration {

template <typename BlockTree>
void saveBlocksAndTip(PopStorage& storage, const BlockTree& tree) {
  storage.saveBlocks<typename BlockTree::index_t>(tree.getBlocks());
  storage.saveTip(*tree.getBestChain().tip());
}

template <typename BlockTree>
bool loadBlockEndorsements(const PopStorage&,
                           const typename BlockTree::index_t&,
                           typename BlockTree::index_t&,
                           BlockTree&,
                           ValidationState&);

template <typename BlockTree>
bool loadAndApplyBlocks(PopStorage& storage,
                        BlockTree& tree,
                        ValidationState& state) {
  auto blocks = storage.loadBlocks<typename BlockTree::index_t>();
  auto tipStored = storage.loadTip<typename BlockTree::index_t>();
  for (const auto& blockPair : blocks) {
    auto* bi = tree.insertBlock(blockPair.second->header);
    bi->status = blockPair.second->status;
    if (!loadBlockEndorsements(storage, *blockPair.second, *bi, tree, state))
      return false;
  }

  auto* tip = tree.getBlockIndex(tipStored.second);
  if (tip == nullptr) return false;
  if (tip->height != tipStored.first) return false;
  return tree.overrideTip(*tip, state);
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKCHAIN_STORAGE_UTIL_HPP_
