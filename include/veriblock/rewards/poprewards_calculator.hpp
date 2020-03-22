#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_params.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculator {
  PopRewardsCalculator() = default;
  virtual ~PopRewardsCalculator(){};

  /**
   * score and difficulty are fractional numbers multiplied to rewardsDecimalsMult
   */
  virtual PopRewardsBigDecimal calculatePopRewardForBlock(
      const AltChainParams& chainParams,
      const PopRewardsParams& rewardParams,
      uint32_t height,
      PopRewardsBigDecimal score,
      PopRewardsBigDecimal difficulty);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
