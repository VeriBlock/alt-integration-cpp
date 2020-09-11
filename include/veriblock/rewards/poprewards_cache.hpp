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
  using pair_t = std::pair<const index_t*, PopRewardsBigDecimal>;

  PopRewardsCache(const AltChainParams& altParams, const VbkBlockTree& vbk_tree)
      : PopRewards(altParams, vbk_tree),
        altParams_(&altParams),
        vbkTree_(&vbk_tree),
        buffer(altParams.getRewardParams().difficultyAveragingInterval() * 2) {}

  virtual ~PopRewardsCache() = default;

  /**
   * Calculate POP difficulty using cache data.
   * Cache should be built using the calculatePayouts() call.
   * @param tip calculate difficulty using chain ending with this tip
   * @return PopRewardsBigDecimal resulting difficulty
   */
  PopRewardsBigDecimal calculateDifficulty(const index_t& tip) const override;

  /**
   * Calculate POP rewards for miners. Rewards are calculated for
   * the endorsed block. This and previous block scores are saved in
   * cache.
   * @param endorsedBlock endorsed altchain block which we are paying reward
   * for.
   * @return std::map<std::vector<uint8_t>, int64_t> map with miner address as a
   * key and reward amount as a value
   */
  std::map<std::vector<uint8_t>, int64_t> calculatePayouts(
      const index_t& endorsedBlock) override;

 protected:
  const AltChainParams* altParams_;
  const VbkBlockTree* vbkTree_;
  ring_buffer<pair_t> buffer;

  PopRewardsBigDecimal updateAndCalculateDifficulty(const index_t& tip);

  bool findCached(const index_t& block, pair_t& out);
  bool findCached(const index_t& block);

  /**
   * Calculate PoP score for the block and append to the cache.
   * @param block calculate score for this block.
   */
  void appendToCache(const index_t& block);

  /**
   * Remove the newest blocks from the cache till the given block
   * is found. Therefore the given block will be the last one in cache,
   * or cache will be empty.
   * @param fromBlock erase cache records starting from this block, not
   * including.
   */
  void truncateCacheHigherThan(const index_t& fromBlock);

  /**
   * Update cache to have scores of this block and any additional blocks
   * required to calculate POP rewards.
   * @param endorsed reward will be paid for this block.
   */
  void updateCache(const index_t& endorsed);

  /**
   * Erase cache and recalculate all missing blocks needed for
   * endorsed block rewards calculation.
   * @param endorsed reward will be paid for this block.
   */
  void rebuildCache(const index_t& endorsed);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
