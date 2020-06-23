// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/btc_blockchain_util.hpp"

#include <chrono>
#include <thread>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

template <>
ArithUint256 getBlockProof(const BtcBlock& block) {
  bool negative = false;
  bool overflow = false;
  auto bnTarget =
      ArithUint256::fromBits(block.getDifficulty(), &negative, &overflow);
  if (negative || overflow || bnTarget == 0) {
    return 0;
  }

  // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
  // as it's too large for an arith_uint256. However, as 2**256 is at least as
  // large as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) /
  // (bnTarget+1)) + 1, or ~bnTarget / (bnTarget+1) + 1.
  return (~bnTarget / (bnTarget + 1)) + 1;
}

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const BlockIndex<BtcBlock>& tip, const merkle_t& merkle) {
  BtcBlock block;
  block.version = tip.header->version;
  block.previousBlock = tip.header->getHash();
  block.merkleRoot = merkle;
  block.timestamp = (std::max)(tip.getBlockTime(), currentTimestamp4());
  block.bits = getNextWorkRequired(tip, block, params_);
  return block;
}

// copied from BTC
static uint32_t calculateNextWorkRequired(
    const BlockIndex<BtcBlock>& currentTip,
    uint32_t nFirstBlockTime,
    const BtcChainParams& params) {
  if (params.getPowNoRetargeting()) {
    return currentTip.getDifficulty();
  }

  auto powTargetTimespan = params.getPowTargetTimespan();

  // Limit adjustment step
  uint32_t nActualTimespan = currentTip.getBlockTime() - nFirstBlockTime;
  if (nActualTimespan < powTargetTimespan / 4) {
    nActualTimespan = powTargetTimespan / 4;
  }
  if (nActualTimespan > powTargetTimespan * 4) {
    nActualTimespan = powTargetTimespan * 4;
  }

  // Retarget
  const ArithUint256 bnPowLimit = params.getPowLimit();
  ArithUint256 bnNew = ArithUint256::fromBits(currentTip.getDifficulty());
  bnNew *= nActualTimespan;
  bnNew /= powTargetTimespan;

  if (bnNew > bnPowLimit) {
    bnNew = bnPowLimit;
  }

  return bnNew.toBits();
}

// copied from BTC
template <>
uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& prevBlock,
                             const BtcBlock& block,
                             const BtcChainParams& params) {
  unsigned int nProofOfWorkLimit = ArithUint256(params.getPowLimit()).toBits();

  // Only change once per difficulty adjustment interval
  if ((prevBlock.height + 1) % params.getDifficultyAdjustmentInterval() != 0) {
    if (params.getAllowMinDifficultyBlocks()) {
      // Special difficulty rule for testnet:
      // If the new block's timestamp is more than 2* 10 minutes
      // then allow mining of a min-difficulty block.
      if (block.timestamp >
          prevBlock.getBlockTime() + params.getPowTargetSpacing() * 2) {
        return nProofOfWorkLimit;
      } else {
        // Return the last non-special-min-difficulty-rules-block
        const BlockIndex<BtcBlock>* pindex = &prevBlock;
        while (pindex->pprev &&
               pindex->height % params.getDifficultyAdjustmentInterval() != 0 &&
               pindex->getDifficulty() == nProofOfWorkLimit)
          pindex = pindex->pprev;
        return pindex->getDifficulty();
      }
    }
    return prevBlock.getDifficulty();
  }

  // Go back by what we want to be 14 days worth of blocks
  uint32_t nHeightFirst =
      prevBlock.height - (params.getDifficultyAdjustmentInterval() - 1);
  const auto* pindexFirst = prevBlock.getAncestor(nHeightFirst);
  if (pindexFirst == nullptr) {
    throw std::logic_error("unable to find block ancestor at given height");
  }

  return calculateNextWorkRequired(
      prevBlock, pindexFirst->getBlockTime(), params);
}

template <>
int64_t getMedianTimePast(const BlockIndex<BtcBlock>& prev) {
  static constexpr int medianTimeSpan = 11;

  int64_t pmedian[medianTimeSpan];
  std::fill(pmedian, pmedian + medianTimeSpan, 0);
  auto* pbegin = &pmedian[medianTimeSpan];
  auto* pend = &pmedian[medianTimeSpan];

  const BlockIndex<BtcBlock>* pindex = &prev;
  for (int i = 0; i < medianTimeSpan && pindex; i++, pindex = pindex->pprev) {
    *(--pbegin) = pindex->getBlockTime();
  }

  std::sort(pbegin, pend);
  return pbegin[(pend - pbegin) / 2];
}

template <>
bool checkBlockTime(const BlockIndex<BtcBlock>& prev,
                    const BtcBlock& block,
                    ValidationState& state) {
  if (int64_t(block.getBlockTime()) < getMedianTimePast(prev)) {
    return state.Invalid("btc-time-too-old", "block's timestamp is too early");
  }

  if (int64_t(block.getBlockTime()) >
      currentTimestamp4() + BTC_MAX_FUTURE_BLOCK_TIME) {
    return state.Invalid("btc-time-too-new",
                         "block timestamp too far in the future");
  }

  return true;
}

template <>
bool contextuallyCheckBlock(const BlockIndex<BtcBlock>& prev,
                            const BtcBlock& block,
                            ValidationState& state,
                            const BtcChainParams& params) {
  if (block.getDifficulty() != getNextWorkRequired(prev, block, params)) {
    return state.Invalid("btc-bad-diffbits",
                         "incorrect proof of work of BTC block");
  }

  if (!checkBlockTime(prev, block, state)) {
    return state.Invalid("btc-check-block-time");
  }

  return true;
}

}  // namespace altintegration
