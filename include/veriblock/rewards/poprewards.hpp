#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/rewards/poprewards_params.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {
/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewards {
  PopRewards(std::shared_ptr<EndorsementRepository<VbkEndorsement>> erepo,
             std::shared_ptr<VbkBlockTree> vbk_tree,
             const PopRewardsParams& rewardParams)
      : erepo_(std::move(erepo)),
        vbk_tree_(std::move(vbk_tree)),
        rewardParams_(rewardParams) {}
  virtual ~PopRewards(){};

  /**
   */
  virtual PopRewardsBigDecimal scoreFromEndorsements(
      const AltBlock& endorsedBlock);

 private:
  std::shared_ptr<EndorsementRepository<VbkEndorsement>> erepo_;
  std::shared_ptr<VbkBlockTree> vbk_tree_;
  PopRewardsParams rewardParams_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
