#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace VeriBlock {

template <>
uint32_t getNextWorkRequired(const BlockIndex<VbkBlock>& currentTip,
                             const VbkBlock& block,
                             const VbkChainParams& params);

template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle);

template <>
ArithUint256 getBlockProof(const VbkBlock& block);

int64_t getMedianTimePast(const BlockIndex<VbkBlock>& prev);

template <>
bool checkBlockTime(const BlockIndex<VbkBlock>& prev,
                    const VbkBlock& block,
                    ValidationState& state);

int64_t calculateMinimumTimestamp(const BlockIndex<VbkBlock>& prev);

bool validateKeystones(const BlockIndex<VbkBlock>& prevBlock,
                       const VbkBlock& block);

template <>
bool contextuallyCheckBlock(const BlockIndex<VbkBlock>& prev,
                            const VbkBlock& block,
                            ValidationState& state,
                            const VbkChainParams& params);

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_
