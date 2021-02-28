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
template <typename T>
class reverse_range {
  T &m_x;

 public:
  explicit reverse_range(T &x) : m_x(x) {}

  auto begin() const -> decltype(this->m_x.rbegin()) { return m_x.rbegin(); }

  auto end() const -> decltype(this->m_x.rend()) { return m_x.rend(); }
};

template <typename T>
reverse_range<T> reverse_iterate(T &x) {
  return reverse_range<T>(x);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_REVERSE_SUBRANGE_HPP
