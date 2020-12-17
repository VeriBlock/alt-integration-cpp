// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculatorInterface {
  virtual ~PopRewardsCalculatorInterface() = default;

  /**
   * Applies penalty for distant endorsements in VBK chain.
   * @param relativeBlock height of the VBK block with endorsement
   * @return PopRewardsBigDecimal multiplier to apply to the block reward
   */
  virtual PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock) const = 0;

  /**
   * Calculate reward for the entire block from the block score, height of the
   * VBK endorsement and current POP difficulty.
   * @param height reward is calculated for the block at this height.
   * @param scoreForThisBlock block score
   * @param difficulty current POP difficulty
   * @return PopRewardsBigDecimal resulting reward for the block
   */
  virtual PopRewardsBigDecimal calculateBlockReward(
      uint32_t height,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const = 0;

  /**
   * Calculate reward for the miner from the block score, height of the VBK
   * endorsement and total block reward.
   * @param vbkRelativeHeight height of the VBK block with endorsement
   * @param scoreForThisBlock block score
   * @param blockReward total block reward
   * @return PopRewardsBigDecimal resulting reward for each miner
   */
  virtual PopRewardsBigDecimal calculateMinerReward(
      uint32_t vbkRelativeHeight,
      const PopRewardsBigDecimal& scoreForThisBlock,
      const PopRewardsBigDecimal& blockReward) const {
    if (scoreForThisBlock == 0.0) {
      return 0.0;
    }
    auto endorsementLevelWeight =
        getScoreMultiplierFromRelativeBlock(vbkRelativeHeight);
    return blockReward * endorsementLevelWeight / scoreForThisBlock;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_
