// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_

#include <veriblock/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculatorInterface {
  virtual ~PopRewardsCalculatorInterface() = default;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block which is PopPayoutDelay blocks behind the tip.
   * @param tip block that contains POP payout. Usually the tip of the
   * active chain.
   * @return PopPayouts map with miner address as a key and reward
   * amount as a value
   */
  virtual PopPayouts getPopPayout(const AltBlockTree::hash_t& tip) = 0;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_INTERFACE_HPP_
