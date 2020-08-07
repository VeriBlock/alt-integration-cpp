// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALGORITHM_HPP
#define VERIBLOCK_POP_CPP_ALGORITHM_HPP

#include <algorithm>
#include <functional>
#include <set>
#include <vector>
#include <veriblock/assert.hpp>
#include <iterator>

namespace altintegration {

// maps one vector into another with function f
template <typename A, typename B>
std::vector<B> map_vector(const std::vector<A>& a,
                          std::function<B(const A&)> f) {
  std::vector<B> b;
  b.reserve(a.size());
  std::transform(a.begin(), a.end(), std::back_inserter(b), f);
  return b;
}

template <typename T>
typename T::id_t get_id(const T& t) {
  return t.getId();
}

template <typename P>
std::vector<typename P::id_t> map_get_id(const std::vector<P>& a) {
  return map_vector<P, typename P::id_t>(a, get_id<P>);
}

template <typename T>
std::set<typename T::id_t> make_idset(const std::vector<T>& v) {
  auto ids = map_get_id(v);
  std::set<typename T::id_t> s(ids.begin(), ids.end());
  return s;
}

template <typename T>
void eraseLastItem(std::vector<T*>& v,
                   const T* item,
                   std::function<bool(const T*)> locator) {
  // find and erase the last occurrence of item
  auto it = std::find_if(v.rbegin(), v.rend(), locator);

  VBK_ASSERT_MSG(it != v.rend(),
                 "failed to remove item from vector: can not find element %s",
                 item->toPrettyString());

  auto toRemove = --(it.base());
  v.erase(toRemove);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALGORITHM_HPP
