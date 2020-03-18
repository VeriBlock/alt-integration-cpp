#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace AltIntegrationLib {

template <>
uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& prevBlock,
                             const BtcBlock& block,
                             const BtcChainParams& params);

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const BlockIndex<BtcBlock>& tip, const merkle_t& merkle);

template <>
ArithUint256 getBlockProof(const BtcBlock& block);

template <>
int64_t getMedianTimePast(const BlockIndex<BtcBlock>& prev);

template <>
bool checkBlockTime(const BlockIndex<BtcBlock>& prev,
                    const BtcBlock& block,
                    ValidationState& state);

template <>
bool contextuallyCheckBlock(const BlockIndex<BtcBlock>& prev,
                            const BtcBlock& block,
                            ValidationState& state,
                            const BtcChainParams& params);

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
