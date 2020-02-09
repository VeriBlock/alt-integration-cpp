#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_

#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/miner.hpp>

namespace VeriBlock {

// copied from BTC
uint32_t calculateNextWorkRequired(const BlockIndex<BtcBlock>& currentTip,
                                   int64_t nFirstBlockTime,
                                   const BtcChainParams& params);

// copied from BTC
uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& currentTip,
                             const BtcBlock& block,
                             const BtcChainParams& params);

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const merkle_t& merkle) const;

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_
