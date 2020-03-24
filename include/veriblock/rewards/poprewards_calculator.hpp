#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_

#include <memory>
#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_params.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculator {
  PopRewardsCalculator(std::shared_ptr<AltChainParams> chainParams,
                       std::shared_ptr<PopRewardsParams> rewardParams)
      : chainParams_(std::move(chainParams)),
        rewardParams_(std::move(rewardParams)) {}
  virtual ~PopRewardsCalculator(){};

  /**
   */
  virtual PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock);

  /**
   */
  virtual PopRewardsBigDecimal calculateRewardForMiner(
      uint32_t height,
      uint32_t vbkRelativeHeight,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty);

 private:
  std::shared_ptr<AltChainParams> chainParams_;
  std::shared_ptr<PopRewardsParams> rewardParams_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
