// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

template <typename BlockTree>
void saveBlocks(PopStorage& storage, const BlockTree& tree) {
  storage.saveBlocks(tree.getBlocks());
  storage.saveTip(*tree.getBestChain().tip());
}

template <typename BlockTree>
bool loadBlocks(const PopStorage& storage,
                BlockTree& tree,
                ValidationState& state);

template <>
bool loadBlocks(const PopStorage& storage,
                AltTree& tree,
                ValidationState& state);

template <>
bool loadBlocks(const PopStorage& storage,
                VbkBlockTree& tree,
                ValidationState& state);

template <>
bool loadBlocks(const PopStorage& storage,
                BlockTree<BtcBlock, BtcChainParams>& tree,
                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_
