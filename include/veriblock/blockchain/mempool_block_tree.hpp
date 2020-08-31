// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP
#define ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP

#include <unordered_map>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/mempool.hpp"

namespace altintegration {

struct MempoolBlockTree {
  using relation_t = MemPool::VbkPayloadsRelations;
  using relations_map_t = std::unordered_map<typename relation_t::id_t,
                                             std::shared_ptr<relation_t>>;

  MempoolBlockTree(const AltTree& tree) : _tree(tree) { (void)_tree; }

  const VbkBlock* getVbkBlock(const typename VbkBlock::hash_t& hash);

 private:
  relations_map_t relations;
  const AltTree& _tree;
};

}  // namespace altintegration

#endif