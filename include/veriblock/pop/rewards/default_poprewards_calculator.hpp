// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_DEFAULT_POPREWARDS_CALCULATOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_DEFAULT_POPREWARDS_CALCULATOR_HPP_

#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/entities/pop_payouts.hpp>

#include "poprewards_bigdecimal.hpp"
#include "poprewards_calculator.hpp"

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct DefaultPopRewardsCalculator : public PopRewardsCalculator {
  using index_t = AltBlockTree::index_t;

  DefaultPopRewardsCalculator(AltBlockTree& tree)
      : tree_(tree) {}
  ~DefaultPopRewardsCalculator() override = default;

  bool getPopPayout(const AltBlockTree::hash_t& tip,
                    PopPayouts& rewards,
                    ValidationState& state) override;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block. Score is calculated from the endorsements (slow).
   * @param endorsedBlock endorsed altchain block which we are paying reward
   * for.
   * @param[out] rewards PopPayouts map with miner address as a key and reward
   * amount as a value.
   * @param[out] state validation state.
   * @return true if rewards are calculated properly; false otherwise.
   */
  bool calculatePayouts(const BlockIndex<AltBlock>& endorsedBlock,
                        PopPayouts& rewards,
                        ValidationState& state);

  /**
   * Collect all endorsements for the endorsed block and calculate
   * POP score
   * @param endorsedBlock calculate score for this block
   * @return PopRewardsBigDecimal resulting score
   */
  PopRewardsBigDecimal scoreFromEndorsements(
      const BlockIndex<AltBlock>& endorsedBlock);

  /**
   * Calculate payout round from the given block height.
   * @param height block height
   * @return uint32_t current round
   */
  uint32_t getRoundForBlockNumber(uint32_t height) const;

  /**
   * Calculate reward for the entire block from the block score, height of the
   * VBK endorsement and current POP difficulty.
   * @param height reward is calculated for the block at this height.
   * @param scoreForThisBlock block score
   * @param difficulty current POP difficulty
   * @return PopRewardsBigDecimal resulting reward for the block
   */
  PopRewardsBigDecimal calculateBlockReward(
      uint32_t height,
      PopRewardsBigDecimal scoreForThisBlock,
      PopRewardsBigDecimal difficulty) const;

  /**
   * Calculate reward for the miner from the block score, height of the VBK
   * endorsement and total block reward.
   * @param vbkRelativeHeight height of the VBK block with endorsement
   * @param scoreForThisBlock block score
   * @param blockReward total block reward
   * @return PopRewardsBigDecimal resulting reward for each miner
   */
  PopRewardsBigDecimal calculateMinerReward(
      uint32_t vbkRelativeHeight,
      const PopRewardsBigDecimal& scoreForThisBlock,
      const PopRewardsBigDecimal& blockReward) const;

 protected:
  AltBlockTree& tree_;

  /**
   * Applies penalty for distant endorsements in VBK chain.
   * @param relativeBlock height of the VBK block with endorsement
   * @return PopRewardsBigDecimal multiplier to apply to the block reward
   */
  PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
      int relativeBlock) const;

  /**
   * Calculate POP difficulty using a list of blocks.
   * Should provide at least rewardSettlementInterval blocks.
   * Will use first blocks (usually difficultyAveragingInterval blocks)
   * to calculate their scores and average them to get difficulty.
   * @param tip calculate difficulty using chain ending with this tip
   * @return PopRewardsBigDecimal resulting difficulty
   */
  PopRewardsBigDecimal calculateDifficulty(const BlockIndex<AltBlock>& tip);

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block. Block score and POP difficulty are set as
   * arguments to allow caching them.
   * @param endorsedBlock endorsed altchain block which we are paying reward
   * for.
   * @param endorsedBlockScore endorsed altchain block score - see
   * scoreFromEndorsements() for reference.
   * @param popDifficulty endorsed altchain block difficulty - see
   * calculateDifficulty() for reference.
   * @param[out] rewards PopPayouts map with miner address as a key and reward
   * amount as a value.
   * @param[out] state validation state.
   * @return true if rewards are calculated properly; false otherwise.
   */
  bool calculatePayoutsInner(const BlockIndex<AltBlock>& endorsedBlock,
                             const PopRewardsBigDecimal& endorsedBlockScore,
                             const PopRewardsBigDecimal& popDifficulty,
                             PopPayouts& rewards,
                             ValidationState& state);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_DEFAULT_POPREWARDS_CALCULATOR_HPP_
