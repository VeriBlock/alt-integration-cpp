#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_FORK_RESOLUTION_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>

namespace VeriBlock {

template <typename Block>
void determineBestChain(Chain<Block>& currentBest, BlockIndex<Block>& indexNew);

}

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_FORK_RESOLUTION_HPP_
