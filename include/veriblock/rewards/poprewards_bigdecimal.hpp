#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_BIGDECIMAL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_BIGDECIMAL_HPP_

#include <vector>
#include <veriblock/arith_uint256.hpp>

namespace altintegration {

struct PopRewardsBigDecimal {
  static const uint32_t decimals = 100000000;
  ArithUint256 value = 0;

  PopRewardsBigDecimal() = default;

  PopRewardsBigDecimal(uint64_t b) : value(ArithUint256(b) * decimals) {}
  PopRewardsBigDecimal(ArithUint256 b) : value(b * decimals) {}
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

  uint64_t getIntegerFraction() { return (value / decimals).getLow64(); }
  uint64_t getDecimalFraction() {
    ArithUint256 integerFraction = getIntegerFraction();
    integerFraction *= decimals;
    return ((value - integerFraction) * decimals).getLow64();
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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_REWARDS_POPREWARDS_BIGDECIMAL_HPP_
