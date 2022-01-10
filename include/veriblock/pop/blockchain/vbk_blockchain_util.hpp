// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/pop/entities/vbkblock.hpp>

#include "blockchain_util.hpp"
#include "chain.hpp"
#include "miner.hpp"
#include "vbk_chain_params.hpp"

namespace altintegration {

//! @private
template <>
uint32_t getNextWorkRequired(const BlockIndex<VbkBlock>& currentTip,
                             const VbkBlock& block,
                             const VbkChainParams& params);

//! @private
template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle);

//! @private
template <>
ArithUint256 getBlockProof(const VbkBlock& block);

//! @private
int64_t getMedianTimePast(const BlockIndex<VbkBlock>& prev);

//! @private
template <>
bool checkBlockTime(const BlockIndex<VbkBlock>& prev,
                    const VbkBlock& block,
                    ValidationState& state,
                    const VbkChainParams& params);

//! @private
int64_t calculateMinimumTimestamp(const BlockIndex<VbkBlock>& prev);

//! @private
bool validateKeystones(const BlockIndex<VbkBlock>& prevBlock,
                       const VbkBlock& block,
                       std::shared_ptr<VbkChainParams> params);

//! @private
template <>
bool contextuallyCheckBlock(const BlockIndex<VbkBlock>& prev,
                            const VbkBlock& block,
                            ValidationState& state,
                            const VbkChainParams& params,
                            bool shouldVerifyNextWork);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCKCHAIN_UTIL_HPP_
