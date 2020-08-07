// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/commands/commands.hpp>

namespace altintegration {

// some explicit template instantiations
template struct AddBlock<BtcBlock, BtcChainParams>;
template struct AddEndorsement<VbkBlockTree::BtcTree, VbkBlockTree>;

template struct AddBlock<VbkBlock, VbkChainParams>;
template struct AddEndorsement<VbkBlockTree, AltTree>;

}  // namespace altintegration