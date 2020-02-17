#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>

namespace VeriBlock {

template <typename Block>
void determineBestChain(Chain<Block>& currentBest, BlockIndex<Block>& indexNew);

template <typename Block, typename ChainParams>
uint32_t getNextWorkRequired(const BlockIndex<Block>& prevBlock,
                             const Block& block,
                             const ChainParams& params);

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
