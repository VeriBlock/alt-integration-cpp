#include <veriblock/third_party/BigDecimal.h>

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
  return target;
}

template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle) {
  VbkBlock block;
  block.version = tip.header.version;
  block.previousBlock =
      tip.header.getHash().trimLE<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>();
  block.merkleRoot = merkle;
  block.height = tip.height + 1;
  // set first previous keystone
  if (block.height >= ALT_KEYSTONE_INTERVAL) {
    auto diff = block.height % ALT_KEYSTONE_INTERVAL;
    diff = diff == 0 ? ALT_KEYSTONE_INTERVAL : diff;
    auto* prevKeystoneIndex = tip.getAncestorBlocksBehind(diff);
    assert(prevKeystoneIndex != nullptr);
    block.previousKeystone =
        prevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();

    // set second previous keystone
    if (block.height >= 2 * ALT_KEYSTONE_INTERVAL) {
      auto* secondPrevKeystoneIndex =
          prevKeystoneIndex->getAncestorBlocksBehind(ALT_KEYSTONE_INTERVAL);
      assert(secondPrevKeystoneIndex != nullptr);
      block.secondPreviousKeystone =
          secondPrevKeystoneIndex->getHash()
              .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
    }
  }
  block.timestamp = startTime_++;
  block.difficulty = getNextWorkRequired(tip, block, *params_);
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

  BCMath::bcscale(15);
  BCMath nextTarget = BCMath(targetDif.toString());
  nextTarget /= (params.getRetargetPeriod() - 1);
  nextTarget.round(8);

  if (t < (int32_t)(K / 10)) {
    t = (int32_t)(K / 10);
  }

  BCMath coef = K;
  coef /= t;
  coef.round(8);

  nextTarget *= coef;

  ArithUint256 minDif = params.getMinimumDifficulty();
  targetDif = ArithUint256::fromString(nextTarget.getIntPart());

  if (targetDif < minDif) {
    return minDif.toBits();
  }

  return targetDif.toBits();
}

}  // namespace VeriBlock
