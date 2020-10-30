// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_

#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/ring_buffer.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCache : public PopRewards {
  using index_t = BlockIndex<AltBlock>;

  PopRewardsCache(const AltChainParams& altParams, const VbkBlockTree& vbk_tree)
      : PopRewards(altParams, vbk_tree),
        altParams_(&altParams),
        vbkTree_(&vbk_tree),
        history_(altParams.getPayoutParams().difficultyAveragingInterval() *
                 2) {}

  virtual ~PopRewardsCache() = default;

  /**
   * Collect all endorsements for the endorsed block and calculate
   * POP score. Use cache if possible.
   * @param endorsedBlock calculate score for this block
   * @return PopRewardsBigDecimal resulting score
   */
  PopRewardsBigDecimal scoreFromEndorsements(
      const index_t& endorsedBlock) override;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block. Score is calculated from cache.
   * @param endorsedBlock endorsed altchain block that we are using to
   * collect endorsements and calculate rewards.
   * @return std::map<std::vector<uint8_t>, int64_t> map with miner address as a
   * key and reward amount as a value
   */
  std::map<std::vector<uint8_t>, int64_t> calculatePayouts(
      const BlockIndex<AltBlock>& endorsedBlock) override;

  /**
   * Erase cache completely.
   */
  void invalidateCache();

  /**
   * Erase N blocks from the cache history.
   * @param blocks amount of blocks to erase
   */
  void eraseCacheHistory(uint32_t blocks);

 protected:
  const AltChainParams* altParams_;
  const VbkBlockTree* vbkTree_;
  ring_buffer<const index_t*> history_;
  std::unordered_map<const index_t*, PopRewardsBigDecimal> cache_;

  /**
   * Calculate PoP score for the block and append to the cache.
   * @param block calculate score for this block.
   * @return PopRewardsBigDecimal score of the block
   */
  PopRewardsBigDecimal appendToCache(const index_t& block);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
