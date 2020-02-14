#include <veriblock/blockchain/btc_blockchain_util.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace VeriBlock {

template <>
BtcBlock Miner<BtcBlock, BtcChainParams>::getBlockTemplate(
    const BlockIndex<BtcBlock>& tip, const merkle_t& merkle) const {
  BtcBlock block;
  block.version = tip.header.version;
  block.previousBlock = tip.header.getHash();
  block.merkleRoot = merkle;
  // TODO: is this correct?
  block.timestamp =
      std::max(currentTimestamp4(), (uint32_t)tip.getMedianTimePast());
  block.bits = getNextWorkRequired(tip, block, *params_);
  return block;
}

template <>
void determineBestChain(Chain<BtcBlock>& currentBest,
                        BlockIndex<BtcBlock>& indexNew) {
  if (currentBest.tip() == nullptr ||
      currentBest.tip()->chainWork < indexNew.chainWork) {
    currentBest.setTip(&indexNew);
  }
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
  assert(pindexFirst);

  return calculateNextWorkRequired(
      prevBlock, pindexFirst->getBlockTime(), params);
}

}  // namespace VeriBlock
