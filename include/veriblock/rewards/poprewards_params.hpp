#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_

#include <vector>

namespace VeriBlock {

// assume 8 decimals for all parameters
static const uint32_t rewardsDecimalsMult = 100000000;

struct PopRewardsCurveParams {
  virtual ~PopRewardsCurveParams() = default;

  // we start decreasing rewards after this score
  virtual uint32_t startOfSlope() const noexcept {
    return (uint32_t)(1.0 * rewardsDecimalsMult);
  }

  // we decrease reward coefficient for this value for
  // each additional score point above startOfDecreasingLine
  virtual uint32_t slopeNormal() const noexcept {
    return (uint32_t)(0.2 * rewardsDecimalsMult);
  }

  virtual uint32_t slopeKeystone() const noexcept {
    return (uint32_t)(0.21325 * rewardsDecimalsMult);
  }
};

struct PopRewardsParams {
  virtual ~PopRewardsParams() = default;

  virtual uint32_t keystoneRound() const noexcept { return 3; }

  virtual uint32_t payoutRounds() const noexcept { return 4; }

  virtual uint32_t flatScoreRound() const noexcept { return 2; }

  virtual bool flatScoreRoundUse() const noexcept { return true; }

  virtual std::vector<uint32_t> roundRatios() const noexcept {
    return {(uint32_t)(0.97 * rewardsDecimalsMult),
            (uint32_t)(1.03 * rewardsDecimalsMult),
            (uint32_t)(1.07 * rewardsDecimalsMult),
            (uint32_t)(3.00 * rewardsDecimalsMult)
    };
  }

  virtual uint32_t maxScoreThresholdNormal() const noexcept {
    return (uint32_t)(2.0 * rewardsDecimalsMult);
  }

  virtual uint32_t maxScoreThresholdKeystone() const noexcept {
    return (uint32_t)(3.0 * rewardsDecimalsMult);
  }

  virtual PopRewardsCurveParams getCurveParams() const noexcept {
    return PopRewardsCurveParams();
  }
};

}  // namespace VeriBlock

#endif // ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
