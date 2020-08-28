// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/storage/util.hpp>

namespace altintegration {

void SaveAllTrees(AltTree& tree, BlockBatchAdaptor& batch) {
  SaveTree(tree.btc(), batch);
  SaveTree(tree.vbk(), batch);
  SaveTree(tree, batch);
}

}  // namespace altintegration