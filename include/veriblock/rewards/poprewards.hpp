#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_

#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/storage/endorsement_repository.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>
#include <veriblock/rewards/poprewards_params.hpp>

namespace altintegration {

struct PopRewardPayout {
  uint64_t reward;
  std::vector<uint8_t> miner;
};

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewards {
  PopRewards(const EndorsementRepository<VbkEndorsement>& erepo,
             const VbkBlockTree& vbk_tree,
             const PopRewardsParams& rewardParams,
             const PopRewardsCalculator& calculator)
      : erepo_(erepo),
        vbk_tree_(vbk_tree),
        rewardParams_(rewardParams),
        calculator_(calculator) {}
  virtual ~PopRewards(){};

  /**
   * Collect all endorsements for the endorsedBlock and calculate
   * POP score
   * @param endorsedBlock calculate score for this block
   * @return PopRewardsBigDecimal resulting score
   */
  virtual PopRewardsBigDecimal scoreFromEndorsements(
      const AltBlock& endorsedBlock);

  /**
   * Calculate POP difficulty using a list of blocks.
   * Should provide at least rewardSettlementInterval blocks.
   * Will use first blocks (usually difficultyAveragingInterval blocks)
   * to calculate their scores and average them to get difficulty.
   * @param blocks calculate difficulty using these blocks
   * @return PopRewardsBigDecimal resulting difficulty
   */
  virtual PopRewardsBigDecimal calculateDifficulty(
      const std::vector<AltBlock>& blocks);

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * a given block.
   * @param endorsedBlock altchain block for which the reward is being paid
   * @param popDifficulty current POP difficulty. See calculateDifficulty for reference.
   * @return std::vector<PopRewardPayout> a list of payouts
   */
  virtual std::vector<PopRewardPayout> calculatePayouts(
      const AltBlock& endorsedBlock,
      PopRewardsBigDecimal popDifficulty);

 private:
  const EndorsementRepository<VbkEndorsement>& erepo_;
  const VbkBlockTree& vbk_tree_;
  PopRewardsParams rewardParams_;
  PopRewardsCalculator calculator_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
