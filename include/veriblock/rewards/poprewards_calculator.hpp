// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_

#include <veriblock/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCalculator {
  virtual ~PopRewardsCalculator() = default;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block which is PopPayoutDelay blocks behind the tip.
   * @param tip last block of the active chain. Next block should contain
   * POP payout calculated with this getPopPayout() call.
   * @return PopPayouts map with miner address as a key and reward
   * amount as a value
   */
  virtual PopPayouts getPopPayout(const AltBlockTree::hash_t& tip) = 0;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CALCULATOR_HPP_
