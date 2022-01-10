// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_RAND_FUZZ_HPP
#define VERIBLOCK_POP_CPP_RAND_FUZZ_HPP

#include <algorithm>
#include <random>
#include <vector>

template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator g) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(g));
  return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end) {
  std::mt19937 m(rand());
  return select_randomly(start, end, m);
}

#endif  // VERIBLOCK_POP_CPP_RAND_FUZZ_HPP
