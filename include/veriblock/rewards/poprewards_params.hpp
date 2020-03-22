#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_

#include <vector>

namespace altintegration {

struct PopRewardsBigDecimal {
  static const uint32_t decimals = 100000000;
  ArithUint256 value = 0;

  PopRewardsBigDecimal() = default;

  PopRewardsBigDecimal(uint64_t b) : value(b) {}
  PopRewardsBigDecimal(ArithUint256 b) : value(b) {}
  PopRewardsBigDecimal(double b) : value((uint64_t)(b * decimals)) {}

  PopRewardsBigDecimal& operator+=(const PopRewardsBigDecimal& b) {
    value += b.value;
    return *this;
  }

  PopRewardsBigDecimal& operator-=(const PopRewardsBigDecimal& b) {
    value -= b.value;
    return *this;
  }

  PopRewardsBigDecimal& operator*=(const PopRewardsBigDecimal& b) {
    value *= b.value;
    value /= decimals;
    return *this;
  }

  PopRewardsBigDecimal& operator/=(const PopRewardsBigDecimal& b) {
    value *= decimals;
    value /= b.value;
    return *this;
  }

  friend inline const PopRewardsBigDecimal operator+(
      const PopRewardsBigDecimal& a, const PopRewardsBigDecimal& b) {
    return PopRewardsBigDecimal(a) += b;
  }
  friend inline const PopRewardsBigDecimal operator-(
      const PopRewardsBigDecimal& a, const PopRewardsBigDecimal& b) {
    return PopRewardsBigDecimal(a) -= b;
  }
  friend inline const PopRewardsBigDecimal operator*(
      const PopRewardsBigDecimal& a, const PopRewardsBigDecimal& b) {
    return PopRewardsBigDecimal(a) *= b;
  }
  friend inline const PopRewardsBigDecimal operator/(
      const PopRewardsBigDecimal& a, const PopRewardsBigDecimal& b) {
    return PopRewardsBigDecimal(a) /= b;
  }
  friend inline bool operator>(const PopRewardsBigDecimal& a,
                               const PopRewardsBigDecimal& b) {
    return a.value.compareTo(b.value) > 0;
  }
  friend inline bool operator<(const PopRewardsBigDecimal& a,
                               const PopRewardsBigDecimal& b) {
    return a.value.compareTo(b.value) < 0;
  }
  friend inline bool operator>=(const PopRewardsBigDecimal& a,
                                const PopRewardsBigDecimal& b) {
    return a.value.compareTo(b.value) >= 0;
  }
  friend inline bool operator<=(const PopRewardsBigDecimal& a,
                                const PopRewardsBigDecimal& b) {
    return a.value.compareTo(b.value) <= 0;
  }
  friend inline bool operator==(const PopRewardsBigDecimal& a,
                                const PopRewardsBigDecimal& b) {
    return a.value.compareTo(b.value) == 0;
  }
};

struct PopRewardsCurveParams {
  virtual ~PopRewardsCurveParams() = default;

  // we start decreasing rewards after this score
  virtual PopRewardsBigDecimal startOfSlope() const noexcept {
    return 1.0;
  }

  // we decrease reward coefficient for this value for
  // each additional score point above startOfDecreasingLine
  virtual PopRewardsBigDecimal slopeNormal() const noexcept {
    return 0.2;
  }

  virtual PopRewardsBigDecimal slopeKeystone() const noexcept {
    return 0.21325;
  }
};

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

  virtual PopRewardsCurveParams getCurveParams() const noexcept {
    return PopRewardsCurveParams();
  }

  // reward score table
  // we score each VeriBlock and lower the reward for late blocks
  virtual std::vector<PopRewardsBigDecimal> relativeScoreLookupTable() const
      noexcept {
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
};

}  // namespace altintegration

#endif // ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_PARAMS_HPP_
