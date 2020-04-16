#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculator {
  PopRewardsCalculator(const AltChainParams& altParams)
      : altParams_(altParams) {}
  virtual ~PopRewardsCalculator(){};

  /**
   * Calculate payout round from the given block height.
   * @param height block height
   * @return uint32_t current round
   */
  virtual uint32_t getRoundForBlockNumber(uint32_t height) const;

  /**
   * Applies penalty for distant endorsements in VBK chain.
   * @param relativeBlock height of the VBK block with endorsement
   * @return PopRewardsBigDecimal multiplier to apply to the block reward
   */
  virtual PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock) const;

  /**
   * Calculate reward for the miner from the block score, height of the VBK
   * endorsement and current POP difficulty.
   * @param height reward is calculated for the block at this height.
   * @param vbkRelativeHeight height of the VBK block with endorsement
   * @param scoreForThisBlock block score
   * @param difficulty current POP difficulty
   * @return PopRewardsBigDecimal resulting reward for each miner
   */
  virtual PopRewardsBigDecimal calculateMinerReward(
      uint32_t height,
      uint32_t vbkRelativeHeight,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const;

  // getter for altchain parameters
  virtual const AltChainParams& getAltParams() const noexcept;

 protected:
  const AltChainParams& altParams_;

  virtual PopRewardsBigDecimal calculateMinerRewardWithWeight(
      uint32_t height,
      PopRewardsBigDecimal endorsementWeight,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
