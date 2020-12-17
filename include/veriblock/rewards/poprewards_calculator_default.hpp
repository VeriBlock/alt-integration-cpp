// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_DEFAULT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_DEFAULT_HPP_

#include <veriblock/rewards/poprewards_calculator_interface.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculatorDefault : public PopRewardsCalculatorInterface {
  PopRewardsCalculatorDefault(const AltChainParams& altParams)
      : altParams_(&altParams) {}
  virtual ~PopRewardsCalculatorDefault() = default;

  /**
   * Calculate payout round from the given block height.
   * @param height block height
   * @return uint32_t current round
   */
  virtual uint32_t getRoundForBlockNumber(uint32_t height) const;

  PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock) const override;

  PopRewardsBigDecimal calculateBlockReward(
      uint32_t height,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const override;

 protected:
  const AltChainParams* altParams_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_DEFAULT_HPP_
