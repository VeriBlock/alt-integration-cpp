// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_REVERSE_SUBRANGE_HPP
#define VERIBLOCK_POP_CPP_REVERSE_SUBRANGE_HPP

#include <vector>

namespace altintegration {

/// translates a pair of forward iterators to a range whose regular iteration
/// order is "backward"
template <typename It>
struct ReversedRange {
  using Rit = std::reverse_iterator<It>;

  ReversedRange(It begin, It end) : begin_(end), end_(begin) {}

  Rit begin() { return begin_; }
  Rit end() { return end_; }

 private:
  Rit begin_;
  Rit end_;
};

template <typename T>
ReversedRange<T> reverse_iterate(T begin, T end) {
  return ReversedRange<T>(begin, end);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_REVERSE_SUBRANGE_HPP
