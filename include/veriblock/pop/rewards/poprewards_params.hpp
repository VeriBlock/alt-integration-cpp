// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_

#include <vector>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/rewards/poprewards_bigdecimal.hpp>

namespace altintegration {

/**
 * A container for curve params.
 */
struct PopRewardsCurveParams {
  virtual ~PopRewardsCurveParams() = default;

  // we start decreasing rewards after this score
  virtual PopRewardsBigDecimal startOfSlope() const noexcept { return 1.0; }

  // we decrease reward coefficient for this value for
  // each additional score point above startOfDecreasingLine
  virtual PopRewardsBigDecimal slopeNormal() const noexcept { return 0.2; }

  virtual PopRewardsBigDecimal slopeKeystone() const noexcept {
    return 0.21325;
  }
};

/**
 * A container for PoP rewards params.
 */
struct PopRewardsParams {
  virtual ~PopRewardsParams() = default;

  virtual uint32_t keystoneRound() const noexcept { return 3; }

  virtual uint32_t payoutRounds() const noexcept { return 4; }

  virtual uint32_t flatScoreRound() const noexcept { return 2; }

  virtual bool flatScoreRoundUse() const noexcept { return true; }

  virtual std::vector<PopRewardsBigDecimal> roundRatios() const noexcept {
    return {0.97, 1.03, 1.07, 3.00};
  }

  virtual PopRewardsBigDecimal maxScoreThresholdNormal() const noexcept {
    return 2.0;
  }

  virtual PopRewardsBigDecimal maxScoreThresholdKeystone() const noexcept {
    return 3.0;
  }

  virtual uint32_t difficultyAveragingInterval() const noexcept { return 50; }

  virtual uint32_t rewardSettlementInterval() const noexcept { return 400; }

  virtual const PopRewardsCurveParams& getCurveParams() const noexcept {
    return curveParms;
  }

  // reward score table
  // we score each VeriBlock and lower the reward for late blocks
  virtual std::vector<PopRewardsBigDecimal> relativeScoreLookupTable()
      const noexcept {
    return {1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
            1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
            1.00000000, 1.00000000, 0.48296816, 0.31551694, 0.23325824,
            0.18453616, 0.15238463, 0.12961255, 0.11265630, 0.09955094,
            0.08912509, 0.08063761, 0.07359692, 0.06766428, 0.06259873,
            0.05822428, 0.05440941, 0.05105386, 0.04807993, 0.04542644,
            0.04304458, 0.04089495, 0.03894540, 0.03716941, 0.03554497,
            0.03405359, 0.03267969, 0.03141000, 0.03023319, 0.02913950,
            0.02812047, 0.02716878, 0.02627801, 0.02544253, 0.02465739,
            0.02391820, 0.02322107, 0.02256255, 0.02193952, 0.02134922};
  }

 protected:
  PopRewardsCurveParams curveParms{};
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
