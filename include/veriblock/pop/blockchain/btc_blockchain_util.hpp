// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/pop/entities/btcblock.hpp>

#include "blockchain_util.hpp"
#include "btc_chain_params.hpp"
#include "chain.hpp"
#include "miner.hpp"

namespace altintegration {

//! @private
template <>
uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& prevBlock,
                             const BtcBlock& block,
                             const BtcChainParams& params);

//! @private
template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const BlockIndex<BtcBlock>& tip, const merkle_t& merkle);

//! @private
template <>
ArithUint256 getBlockProof(const BtcBlock& block);

//! @private
template <>
int64_t getMedianTimePast(const BlockIndex<BtcBlock>& prev);

//! @private
template <>
bool checkBlockTime(const BlockIndex<BtcBlock>& prev,
                    const BtcBlock& block,
                    ValidationState& state,
                    const BtcChainParams& param);

//! @private
template <>
bool contextuallyCheckBlock(const BlockIndex<BtcBlock>& prev,
                            const BtcBlock& block,
                            ValidationState& state,
                            const BtcChainParams& params,
                            bool shouldVerifyNextWork);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_BLOCKCHAIN_UTIL_HPP_
