#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/vbkblock.hpp"

namespace VeriBlock {

template <>
ArithUint256 getBlockProof(const VbkBlock& block) {
  bool negative = false;
  bool overflow = false;
  auto target = ArithUint256::fromBits(block.difficulty, &negative, &overflow);
  if (negative || overflow || target == 0) {
    return 0;
  }

  // TODO
  return target;
}

template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle) const {
  VbkBlock block;
  block.version = tip.header.version;
  block.previousBlock =
      tip.header.getHash().trimLE<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>();
  block.merkleRoot = merkle;
  block.height = tip.height + 1;
  // set first previous keystone
  if (block.height >= KEYSTONE_INTERVAL) {
    auto diff = block.height % KEYSTONE_INTERVAL;
    diff = diff == 0 ? KEYSTONE_INTERVAL : diff;
    auto* prevKeystoneIndex = tip.getAncestorBlocksBehind(diff);
    assert(prevKeystoneIndex != nullptr);
    block.previousKeystone =
        prevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();

    // set second previous keystone
    if (block.height >= 2 * KEYSTONE_INTERVAL) {
      auto* secondPrevKeystoneIndex =
          prevKeystoneIndex->getAncestorBlocksBehind(KEYSTONE_INTERVAL);
      assert(secondPrevKeystoneIndex != nullptr);
      block.secondPreviousKeystone =
          secondPrevKeystoneIndex->getHash()
              .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
    }
  }
  block.timestamp = currentTimestamp4();
  block.difficulty = getNextWorkRequired(tip, block, *params_);
  return block;
}

template <>
void determineBestChain(Chain<VbkBlock>& currentBest,
                        BlockIndex<VbkBlock>& indexNew) {
  // It is a temprorary solution, it has been copied from the Btc implementation
  if (currentBest.tip() == nullptr ||
      currentBest.tip()->chainWork < indexNew.chainWork) {
    currentBest.setTip(&indexNew);
  }
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

  ArithUint256 nextTarget;
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

    nextTarget += ArithUint256::fromBits(workBlock->pprev->getDifficulty());
  }

  nextTarget *= 1000000000;
  nextTarget /= (params.getRetargetPeriod() - 1);
  // Half up rounding
  nextTarget += 5;
  nextTarget /= 1000000000;

  if (t < (int32_t)(K / 10)) {
    t = (int32_t)(K / 10);
  }

  double coef = (double)K / t;
  // Half up rounding
  coef += 0.000000005;

  nextTarget *= (uint32_t)(coef * 100000000);
  nextTarget /= 100000000;

  ArithUint256 minDif = params.getMinimumDifficulty();

  if (nextTarget < minDif) {
    return minDif.toBits();
  }

  return nextTarget.toBits();
}

}  // namespace VeriBlock
