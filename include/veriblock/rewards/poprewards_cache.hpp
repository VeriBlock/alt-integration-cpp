// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/ring_buffer.hpp>

namespace altintegration {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCache : public PopRewards {
  using index_t = BlockIndex<AltBlock>;
  using pair_t = std::pair<index_t, PopRewardsBigDecimal>;

  PopRewardsCache(const AltChainParams& altParams, const VbkBlockTree& vbk_tree)
      : PopRewards(altParams, vbk_tree),
        altParams_(&altParams),
        vbkTree_(&vbk_tree),
        buffer(altParams.getRewardParams().difficultyAveragingInterval() * 2) {}

  virtual ~PopRewardsCache() = default;

  virtual PopRewardsBigDecimal updateAndCalculateDifficulty(const index_t& tip);

  PopRewardsBigDecimal calculateDifficulty(
      const BlockIndex<AltBlock>& tip) const override;

  std::map<std::vector<uint8_t>, int64_t> calculatePayouts(
      const index_t& endorsedBlock,
      const PopRewardsBigDecimal& popDifficulty) override;

 protected:
  const AltChainParams* altParams_;
  const VbkBlockTree* vbkTree_;
  ring_buffer<pair_t> buffer;

  bool cutend(const index_t& block);

  void appendEndorsed(const index_t& block);

  void updateCached(const index_t& endorsed);

  void invalidateCache(const index_t& endorsed);

  bool cacheExists(const index_t* range);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
