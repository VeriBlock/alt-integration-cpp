// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_HPP

#include <veriblock/pop/blockchain/alt_block_tree.hpp>

namespace altintegration {

struct AltBlockTreeFacade {
  AltBlockTreeFacade(AltBlockTree& tree) : alt_(&tree) {}



 private:
  AltBlockTree* alt_ = nullptr;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_HPP
