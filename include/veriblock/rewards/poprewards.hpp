#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/rewards/poprewards_params.hpp>

namespace VeriBlock {
/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewards {
  PopRewards() = default;
  virtual ~PopRewards(){};

  // assume 8 decimals for reward payouts
  //static const uint32_t rewardDecimalsMult = 100000000;

  /**
   */
  // we calculate the reward for a given block
  virtual ArithUint256 calculatePopRewardForBlock(
      const AltChainParams& chainParams,
      const PopRewardsParams& rewardParams,
      uint32_t height,
      ArithUint256 scoreForThisBlock,
      ArithUint256 difficulty);

 private:
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_HPP_
