// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mempool_prioritization.hpp"

namespace altintegration {

bool isStronglyEquivalent(const VTB& vtb1,
                          const VTB& vtb2,
                          VbkBlockTree& tree) {
  auto* vtb1_containing_block =
      tree.getBlockIndex(vtb1.containingBlock.getHash());
  auto* vtb2_containing_block =
      tree.getBlockIndex(vtb2.containingBlock.getHash());

  bool is_on_the_same_chain = false;
  if (vtb1_containing_block->getHeight() > vtb2_containing_block->getHeight()) {
    is_on_the_same_chain =
        vtb1_containing_block->getAncestor(
            vtb2_containing_block->getHeight()) == vtb2_containing_block;
  } else {
    is_on_the_same_chain =
        vtb2_containing_block->getAncestor(
            vtb1_containing_block->getHeight()) == vtb1_containing_block;
  }

  return (vtb1.transaction.bitcoinTransaction ==
          vtb2.transaction.bitcoinTransaction) &&
         (vtb1.transaction.blockOfProof == vtb1.transaction.blockOfProof) &&
         is_on_the_same_chain;
}

}  // namespace altintegration