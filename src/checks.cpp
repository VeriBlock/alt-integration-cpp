// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/checks.hpp>
#include <veriblock/fmt.hpp>

namespace altintegration {

void checkRange(int64_t num, int64_t min, int64_t max) {
  if (num < min) {
    throw std::out_of_range("value is less than minimal");
  }

  if (num > max) {
    throw std::out_of_range("value is greater than maximum");
  }
}

bool checkRange(int64_t num, int64_t min, int64_t max, ValidationState& state) {
  if (num < min) {
    return state.Invalid(
        "range-below",
        fmt::format(
            "Expected num to be more or equal to {}, but got {}", min, num));
  }

  if (num > max) {
    return state.Invalid(
        "range-above",
        fmt::format(
            "Expected num to be less or equal than {}, but got {}", max, num));
  }
  return true;
}

}  // namespace altintegration
