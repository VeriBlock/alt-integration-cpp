// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATE_UTILS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATE_UTILS_HPP_

#include <algorithm>
#include <memory>
#include <vector>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/state_manager.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

template <typename Block_t, typename ChainParams_t>
bool loadBlockTree(
    BlockTree<Block_t, ChainParams_t>& tree,
    std::shared_ptr<typename BlockRepository<BlockIndex<Block_t>>::cursor_t>
        cursor,
    ValidationState& state) {
  std::vector<BlockIndex<Block_t>> blocks;

  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    blocks.push_back(cursor->value());
  }

  std::sort(blocks.begin(),
            blocks.end(),
            [](const BlockIndex<Block_t>& block1,
               const BlockIndex<Block_t>& block2) -> bool {
              return block1.height < block2.height;
            });

  for (const auto& block : blocks) {
    if (!tree.acceptBlock(block.header, state)) {
      return state.addStackFunction("loadBlockTree()");
    }
  }

  return true;
}

}  // namespace altintegration

#endif
