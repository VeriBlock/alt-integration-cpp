#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

template <typename Block>
void determineBestChain(Chain<Block>& currentBest, BlockIndex<Block>& indexNew);

template <typename Block, typename ChainParams>
uint32_t getNextWorkRequired(const BlockIndex<Block>& prevBlock,
                             const Block& block,
                             const ChainParams& params);

template <typename Block>
ArithUint256 getBlockProof(const Block& block);

template <typename Block>
int64_t getMedianTimePast(const BlockIndex<Block>& prev);

template <typename Block>
bool checkBlockTime(const BlockIndex<Block>& prev,
                    const Block& block,
                    ValidationState& state);

template <typename Block, typename ChainParams>
bool contextuallyCheckBlock(const BlockIndex<Block>& prev,
                            const Block& block,
                            ValidationState& state,
                            const ChainParams& params);

template <typename BlockTree, typename BlockIndexT>
void addContextToBlockIndex(BlockIndexT&,
                            const typename BlockIndexT::payloads_t&,
                            const BlockTree& tree);
/*
template <typename BlockIndexT>
void removeContextFromBlockIndex(BlockIndexT&,
                                 const typename BlockIndexT::payloads_t&);*/

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
