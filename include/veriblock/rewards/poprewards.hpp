// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_

#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/rewards/poprewards_bigdecimal.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewards {
  PopRewards(const AltChainParams& altParams) : calculator_(altParams) {}

  virtual ~PopRewards() = default;

  /**
   * Collect all endorsements for the endorsed block and calculate
   * POP score
   * @param endorsedBlock calculate score for this block
   * @return PopRewardsBigDecimal resulting score
   */
  virtual PopRewardsBigDecimal scoreFromEndorsements(
      const VbkBlockTree& vbk_tree,
      const BlockIndex<AltBlock>& endorsedBlock) const;

  /**
   * Calculate POP difficulty using a list of blocks.
   * Should provide at least rewardSettlementInterval blocks.
   * Will use first blocks (usually difficultyAveragingInterval blocks)
   * to calculate their scores and average them to get difficulty.
   * @param tip calculate difficulty using chain ending with this tip
   * @return PopRewardsBigDecimal resulting difficulty
   */
  virtual PopRewardsBigDecimal calculateDifficulty(
      const VbkBlockTree& vbk_tree, const BlockIndex<AltBlock>& tip) const;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block.
   * @param endorsedBlock endorsed altchain block which we are paying reward
   * for.
   * @param popDifficulty current POP difficulty. See calculateDifficulty for
   * reference.
   * @return std::map<std::vector<uint8_t>, int64_t> map with miner address as a
   * key and reward amount as a value
   */
  virtual std::map<std::vector<uint8_t>, int64_t> calculatePayouts(
      const VbkBlockTree& vbk_tree,
      const BlockIndex<AltBlock>& endorsedBlock,
      const PopRewardsBigDecimal& popDifficulty);

 private:
  PopRewardsCalculator calculator_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
