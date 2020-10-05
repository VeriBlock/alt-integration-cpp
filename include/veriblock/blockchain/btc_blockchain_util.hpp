// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

template <>
uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& prevBlock,
                             const BtcBlock& block,
                             const BtcChainParams& params);

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const BlockIndex<BtcBlock>& tip,
    const Miner<BtcBlock, BtcChainParams>::merkle_t& merkle);

template <>
ArithUint256 getBlockProof(const BtcBlock& block);

template <>
int64_t getMedianTimePast(const BlockIndex<BtcBlock>& prev);

template <>
bool checkBlockTime(const BlockIndex<BtcBlock>& prev,
                    const BtcBlock& block,
                    ValidationState& state,
                    const BtcChainParams& param);

template <>
bool contextuallyCheckBlock(const BlockIndex<BtcBlock>& prev,
                            const BtcBlock& block,
                            ValidationState& state,
                            const BtcChainParams& params,
                            bool shouldVerifyNextWork);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
