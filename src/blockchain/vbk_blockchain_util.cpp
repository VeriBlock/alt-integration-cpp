#include "veriblock/blockchain/vbk_blockchain_util.hpp"

#include <veriblock/third_party/BigDecimal.h>

#include "veriblock/arith_uint256.hpp"
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
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle) const {
  VbkBlock block;
  block.version = tip.header.version;
  block.previousBlock =
      tip.header.getHash().trimLE<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>();
  block.merkleRoot = merkle;
  block.height = tip.height + 1;
  // set first previous keystone
  auto diff = tip.height % VBK_KEYSTONE_INTERVAL;

  // we do not use previous block as a keystone
  if (diff == 0) {
    diff += VBK_KEYSTONE_INTERVAL;
  }
  // we reference genesis block if we are at the beginning of the chain
  if (diff <= tip.height) {
    auto* prevKeystoneIndex = tip.getAncestor(tip.height - diff);
    assert(prevKeystoneIndex != nullptr);
    block.previousKeystone =
        prevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
  } else {
    block.previousKeystone = VbkBlock::keystone_t();
  }

  // set second previous keystone
  diff += VBK_KEYSTONE_INTERVAL;
  if (diff <= tip.height) {
    auto* secondPrevKeystoneIndex = tip.getAncestor(tip.height - diff);
    assert(secondPrevKeystoneIndex != nullptr);
    block.secondPreviousKeystone =
        secondPrevKeystoneIndex->getHash()
            .template trimLE<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
  } else {
    block.secondPreviousKeystone = VbkBlock::keystone_t();
  }

  block.timestamp = currentTimestamp4();
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

template <>
bool BlockTree<VbkBlock, VbkChainParams>::validateKeystones(
    const BlockIndex<VbkBlock>& prevBlock, const VbkBlock& block) const {
  auto tipHeight = prevBlock.height;
  auto diff = tipHeight % VBK_KEYSTONE_INTERVAL;

  // we do not use previous block as a keystone
  if (diff == 0) {
    diff += VBK_KEYSTONE_INTERVAL;
  }
  if (diff <= tipHeight) {
    auto* prevKeystoneIndex = prevBlock.getAncestor(tipHeight - diff);
    if (prevKeystoneIndex == nullptr) return false;

    if (prevKeystoneIndex->getHash().trimLE<VbkBlock::keystone_t::size()>() !=
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
  diff += VBK_KEYSTONE_INTERVAL;
  if (diff <= tipHeight) {
    auto* secondPrevKeystoneIndex = prevBlock.getAncestor(tipHeight - diff);
    if (secondPrevKeystoneIndex == nullptr) return false;

    if (secondPrevKeystoneIndex->getHash()
            .trimLE<VbkBlock::keystone_t::size()>() !=
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
  int64_t median = getMedianTimePast(prev);
  if (int64_t(block.getBlockTime()) < median) {
    return state.Invalid(
        "checkBlockTime()", "time-too-old", "block's timestamp is too early");
  }

  // TODO: find out the max future block time for VBK
  if (int64_t(block.getBlockTime()) >
      currentTimestamp4() + BTC_MAX_FUTURE_BLOCK_TIME) {
    return state.Invalid("checkBlockTime()",
                         "time-too-new",
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
  for (i = 0; i < HISTORY_FOR_TIMESTAMP_AVERAGE; i++, pindex = pindex->pprev) {
    if (pindex == nullptr) {
      break;
    }
    pmedian.push_back(pindex->getBlockTime());
  }

  assert(i > 0);
  assert(!pmedian.empty());
  std::sort(pmedian.begin(), pmedian.end());
  size_t index = i % 2 == 0 ? (i / 2) - 1 : (i / 2);
  return pmedian.at(index);
}

}  // namespace VeriBlock
