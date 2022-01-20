// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALGORITHM_HPP
#define VERIBLOCK_POP_CPP_ALGORITHM_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <unordered_set>
#include <vector>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/blob.hpp>

namespace altintegration {

//! @private
template <typename A, typename B>
std::vector<B> map_vector(const std::vector<A>& a,
                          std::function<B(const A&)> f) {
  std::vector<B> b;
  b.reserve(a.size());
  std::transform(a.begin(), a.end(), std::back_inserter(b), f);
  return b;
}

//! @private
template <typename T>
typename T::id_t get_id(const T& t) {
  return t.getId();
}

//! @private
template <typename T>
std::vector<uint8_t> getIdVector(const T& t) {
  auto id = get_id<T>(t);
  std::vector<uint8_t> v(id.begin(), id.end());
  return v;
}

//! @private
template <size_t N>
std::vector<uint8_t> getIdVector(const Blob<N>& t) {
  std::vector<uint8_t> v(t.begin(), t.end());
  return v;
}

//! @private
template <typename P>
std::vector<typename P::id_t> map_get_id(const std::vector<P>& a) {
  return map_vector<P, typename P::id_t>(a, get_id<P>);
}

//! @private
template <typename A, typename B>
std::vector<A> map_get_id_from_pointers(const std::vector<B*>& b) {
  std::vector<A> a;
  a.reserve(b.size());
  std::transform(b.begin(), b.end(), std::back_inserter(a), [&](B* t) -> A {
    return t->getId();
  });
  return a;
}

//! @private
template <typename T>
bool same_vectors_unique_unordered(const std::vector<T>& a,
                                   const std::vector<T>& b) {
  const auto setA = std::set<T>(a.begin(), a.end());
  const auto setB = std::set<T>(b.begin(), b.end());
  return setA == setB;
}

//! @private
template <typename T>
bool same_vectors_unordered(const std::vector<T>& a,
                            const std::vector<T>& b) {

  std::vector<T> sortedVectorA = a;
  std::vector<T> sortedVectorB = b;
  std::sort(sortedVectorA.begin(), sortedVectorA.end());
  std::sort(sortedVectorB.begin(), sortedVectorB.end());
  return sortedVectorA == sortedVectorB;
}

//! @private
template <typename T>
std::set<typename T::id_t> make_idset(const std::vector<T>& v) {
  auto ids = map_get_id(v);
  std::set<typename T::id_t> s(ids.begin(), ids.end());
  return s;
}

//! @private
template <typename T>
bool erase_last_item_if(std::vector<T>& v,
                        const std::function<bool(const T&)>& locator) {
  // find and erase the last occurrence of item
  size_t last = v.size();
  for (size_t i = v.size(); i-- > 0;) {
    const T& el = v[i];
    if (locator(el)) {
      last = i;
      break;
    }
  }

  if (last >= v.size()) {
    // not found
    return false;
  }

  auto it = v.begin() + last;
  v.erase(it);

  return true;
}

//! removes all elements from container, which satisfy given predicate
//! @private
template <typename container_t, typename val_t>
void erase_if(container_t& c, std::function<bool(const val_t&)> pred) {
  for (auto it = c.begin(); it != c.end();) {
    it = pred(*it) ? c.erase(it) : std::next(it);
  }
}

//! it's a shame that C++11 does not have std::make_unique
//! @private
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  auto* ptr = new T(std::forward<Args...>(args)...);
  return std::unique_ptr<T>(ptr);
}

//! @private
template <typename T>
const T& as_const(const T& t) {
  return t;
}

//! @private
template <typename T>
T& as_mut(const T& t) {
  return const_cast<T&>(t);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALGORITHM_HPP
