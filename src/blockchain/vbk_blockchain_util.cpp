// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/vbk_blockchain_util.hpp"

#include <veriblock/third_party/BigDecimal.h>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/vbkblock.hpp"

namespace altintegration {

template <>
ArithUint256 getBlockProof(const VbkBlock& block) {
  bool negative = false;
  bool overflow = false;
  auto target = ArithUint256::fromBits(block.difficulty, &negative, &overflow);
  if (negative || overflow || target == 0) {
    return 0;
  }
  return target;
}

template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle) {
  VbkBlock block;
  block.version = tip.header->version;
  block.previousBlock =
      tip.header->getHash().template trimLE<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>();
  block.merkleRoot = merkle;
  block.height = tip.height + 1;
  // set first previous keystone
  auto diff = tip.height % params_.getKeystoneInterval();

  // we do not use previous block as a keystone
  if (diff == 0) {
    diff += params_.getKeystoneInterval();
  }
  // we reference genesis block if we are at the beginning of the chain
  if ((int32_t)diff <= tip.height) {
    auto* prevKeystoneIndex = tip.getAncestor(tip.height - diff);
    VBK_ASSERT(prevKeystoneIndex != nullptr);
    block.previousKeystone =
        prevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
  }

  // set second previous keystone
  diff += params_.getKeystoneInterval();
  if ((int32_t)diff <= tip.height) {
    auto* secondPrevKeystoneIndex = tip.getAncestor(tip.height - diff);
    VBK_ASSERT(secondPrevKeystoneIndex != nullptr);
    block.secondPreviousKeystone =
        secondPrevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
  }

  block.timestamp = (std::max)(tip.getBlockTime(), currentTimestamp4());
  block.difficulty = getNextWorkRequired(tip, block, params_);
  return block;
}

template <>
uint32_t getNextWorkRequired(const BlockIndex<VbkBlock>& prevBlock,
                             const VbkBlock&,
                             const VbkChainParams& params) {
  static const uint32_t K = params.getRetargetPeriod() *
                            (params.getRetargetPeriod() - 1) *
                            params.getTargetBlockTime() / 2;

  if (params.getPowNoRetargeting() ||
      (uint32_t)prevBlock.height < params.getRetargetPeriod()) {
    return prevBlock.getDifficulty();
  }

  ArithUint256 targetDif = 0;
  int32_t t = 0;
  uint32_t i = 0;

  for (const BlockIndex<VbkBlock>* workBlock = &prevBlock;
       i < params.getRetargetPeriod() - 1 && workBlock->pprev != nullptr;
       ++i, workBlock = workBlock->pprev) {
    int32_t solveTime =
        workBlock->getBlockTime() - workBlock->pprev->getBlockTime();

    if (solveTime > (int32_t)(params.getTargetBlockTime() * 6)) {
      solveTime = params.getTargetBlockTime() * 6;
    } else if (solveTime < -6 * (int32_t)params.getTargetBlockTime()) {
      solveTime = -6 * (int32_t)params.getTargetBlockTime();
    }

    t += solveTime * (params.getRetargetPeriod() - i - 1);

    targetDif += ArithUint256::fromBits(workBlock->pprev->getDifficulty());
  }

  targetDif *= 1000000000;
  targetDif /= (params.getRetargetPeriod() - 1);
  // Half up rounding
  targetDif += 5;

  if (t < (int32_t)(K / 10)) {
    t = (int32_t)(K / 10);
  }

  double coef2 = (double)K / t;
  // Half up rounding
  coef2 += 0.000000005;

  targetDif *= (uint32_t)(coef2 * 100000000);
  targetDif /= 1000000000;
  targetDif /= 100000000;

  ArithUint256 minDif = params.getMinimumDifficulty();

  if (targetDif < minDif) {
    return minDif.toBits();
  }

  return targetDif.toBits();
}

bool validateKeystones(const BlockIndex<VbkBlock>& prevBlock,
                       const VbkBlock& block,
                       const VbkChainParams& params) {
  auto tipHeight = prevBlock.height;
  auto diff = tipHeight % params.getKeystoneInterval();

  // we do not use previous block as a keystone
  if (diff == 0) {
    diff += params.getKeystoneInterval();
  }
  if ((int32_t)diff <= tipHeight) {
    auto* prevKeystoneIndex = prevBlock.getAncestor(tipHeight - diff);
    if (prevKeystoneIndex == nullptr) {
      return false;
    }

    if (prevKeystoneIndex->getHash()
            .template trimLE<VbkBlock::keystone_t::size()>() !=
        block.previousKeystone) {
      return false;
    }
  } else {
    // should contain zeroes
    if (block.previousKeystone != VbkBlock::keystone_t()) {
      return false;
    }
  }

  // set second previous keystone
  diff += params.getKeystoneInterval();
  if ((int32_t)diff <= tipHeight) {
    auto* secondPrevKeystoneIndex = prevBlock.getAncestor(tipHeight - diff);
    if (secondPrevKeystoneIndex == nullptr) {
      return false;
    }

    if (secondPrevKeystoneIndex->getHash()
            .template trimLE<VbkBlock::keystone_t::size()>() !=
        block.secondPreviousKeystone) {
      return false;
    }
  } else {
    // should contain zeroes
    if (block.secondPreviousKeystone != VbkBlock::keystone_t()) {
      return false;
    }
  }

  return true;
}

int64_t getMedianTimePast(const BlockIndex<VbkBlock>& prev) {
  // height of block to be added is prev.height + 1

  // at block 110000 VBK enables different algorithm of median time calculation,
  // which is implemented in `calculateMinimumTimestamp`. if you even encounter
  // time error on legacy (pre-110000) VBK mainnet/testnet blocks, you will need
  // to add `calculateMinimumTimestampLegacy` from VBK here.
  return calculateMinimumTimestamp(prev);
}

template <>
bool checkBlockTime(const BlockIndex<VbkBlock>& prev,
                    const VbkBlock& block,
                    ValidationState& state) {
  int64_t blockTime = block.getBlockTime();
  int64_t median = getMedianTimePast(prev);
  if (blockTime < median) {
    return state.Invalid("vbk-time-too-old", "block's timestamp is too early");
  }

  int64_t maxTime = currentTimestamp4() + VBK_MAX_FUTURE_BLOCK_TIME;
  if (blockTime > maxTime) {
    return state.Invalid("vbk-time-too-new",
                         "block timestamp too far in the future");
  }

  return true;
}

int64_t calculateMinimumTimestamp(const BlockIndex<VbkBlock>& prev) {
  // Calculate the MEDIAN. If there are an even number of elements,
  // use the lower of the two.

  size_t i = 0;
  std::vector<int64_t> pmedian;
  const BlockIndex<VbkBlock>* pindex = &prev;
  for (i = 0; i < HISTORY_FOR_TIMESTAMP_AVERAGE;
       i++, pindex = pindex->pprev) {
    if (pindex == nullptr) {
      break;
    }
    pmedian.push_back(pindex->getBlockTime());
  }

  VBK_ASSERT(i > 0);
  VBK_ASSERT(!pmedian.empty());
  std::sort(pmedian.begin(), pmedian.end());
  size_t index = i % 2 == 0 ? (i / 2) - 1 : (i / 2);
  return pmedian.at(index);
}

template <>
bool contextuallyCheckBlock(const BlockIndex<VbkBlock>& prev,
                            const VbkBlock& block,
                            ValidationState& state,
                            const VbkChainParams& params) {
  if (!checkBlockTime(prev, block, state)) {
    return state.Invalid("vbk-check-block-time");
  }

  if (block.getDifficulty() != getNextWorkRequired(prev, block, params)) {
    return state.Invalid("vbk-bad-diffbits", "incorrect proof of work");
  }

  // check keystones
  if (!validateKeystones(prev, block, params)) {
    return state.Invalid("vbk-bad-keystones", "incorrect keystones");
  }

  return true;
}

}  // namespace altintegration
