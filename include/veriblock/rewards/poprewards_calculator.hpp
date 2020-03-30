#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_params.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculator {
  PopRewardsCalculator(const AltChainParams& chainParams,
                       const PopRewardsParams& rewardParams)
      : chainParams_(chainParams),
        rewardParams_(rewardParams) {}
  virtual ~PopRewardsCalculator(){};

  /**
   */
  virtual PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock) const;

  /**
   */
  virtual PopRewardsBigDecimal calculateRewardForMiner(
      uint32_t height,
      uint32_t vbkRelativeHeight,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const;

 private:
  const AltChainParams& chainParams_;
  const PopRewardsParams& rewardParams_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
