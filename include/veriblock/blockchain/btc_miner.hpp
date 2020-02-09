#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_

#include <veriblock/blockchain/miner.hpp>

namespace VeriBlock {

uint32_t calculateNextWorkRequired(const BlockIndex<BtcBlock>& currentTip,
                                   int64_t nFirstBlockTime,
                                   const BtcChainParams& params) {
  if (params.getPowNoRetargeting()) {
    return currentTip.getDifficulty();
  }

  auto powTargetTimespan = params.getPowTargetTimespan();

  // Limit adjustment step
  int64_t nActualTimespan = currentTip.getBlockTime() - nFirstBlockTime;
  if (nActualTimespan < powTargetTimespan / 4) {
    nActualTimespan = powTargetTimespan / 4;
  }
  if (nActualTimespan > powTargetTimespan * 4) {
    nActualTimespan = powTargetTimespan * 4;
  }

  // Retarget
  const ArithUint256 bnPowLimit = params.getPowLimit();
  ArithUint256 bnNew;
  bnNew.encodeBits(currentTip.getDifficulty());
  bnNew *= nActualTimespan;
  bnNew /= powTargetTimespan;

  if (bnNew > bnPowLimit) {
    bnNew = bnPowLimit;
  }

  return bnNew.encodeBits();
}

uint32_t getNextWorkRequired(const BlockIndex<BtcBlock>& currentTip,
                             const BtcBlock& block,
                             const BtcChainParams& params) {
  unsigned int nProofOfWorkLimit =
      ArithUint256(params.getPowLimit()).encodeBits();

  // Only change once per difficulty adjustment interval
  if ((currentTip.height + 1) % params.getDifficultyAdjustmentInterval() != 0) {
    if (params.getAllowMinDifficultyBlocks()) {
      // Special difficulty rule for testnet:
      // If the new block's timestamp is more than 2* 10 minutes
      // then allow mining of a min-difficulty block.
      if (block.timestamp >
          currentTip.getBlockTime() + params.getPowTargetSpacing() * 2) {
        return nProofOfWorkLimit;
      } else {
        // Return the last non-special-min-difficulty-rules-block
        const BlockIndex<BtcBlock>* pindex = &currentTip;
        while (pindex->pprev &&
               pindex->height % params.getDifficultyAdjustmentInterval() != 0 &&
               pindex->getDifficulty() == nProofOfWorkLimit)
          pindex = pindex->pprev;
        return pindex->getDifficulty();
      }
    }
    return currentTip.getDifficulty();
  }

  // Go back by what we want to be 14 days worth of blocks
  int nHeightFirst =
      currentTip.height - (params.getDifficultyAdjustmentInterval() - 1);
  assert(nHeightFirst >= 0);
  const auto* pindexFirst = currentTip.getAncestor(nHeightFirst);
  assert(pindexFirst);

  return calculateNextWorkRequired(
      currentTip, pindexFirst->getBlockTime(), params);
}

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const merkle_t& merkle) const {
  auto& bestChain = this->blockchain_->getBestChain();
  auto* tip = bestChain.tip();
  if (tip == nullptr) {
    throw std::logic_error(
        "miner attempted to create block template, but blockchain is not "
        "bootstrapped");
  }

  BtcBlock block;
  block.version = tip->header.version;
  block.previousBlock = tip->header.getHash();
  block.merkleRoot = merkle;
  block.timestamp =
      std::max(currentTimestamp4(), (uint32_t)tip->getMedianTimePast());
  block.bits = getNextWorkRequired(*tip, block, *params_);
  return block;
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_MINER_HPP_
