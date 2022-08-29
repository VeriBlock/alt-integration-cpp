// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_E2E_RAND_HPP
#define TEST_E2E_RAND_HPP

#include <algorithm>
#include <random>

namespace altintegration {
namespace testing_utils {

template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator g) {
  std::uniform_int_distribution<> dis(
      0, static_cast<int>(std::distance(start, end) - 1));
  std::advance(start, dis(g));
  return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end) {
  std::mt19937 m(rand());
  return select_randomly(start, end, m);
}

}  // namespace testing_utils
}  // namespace altintegration

#endif
