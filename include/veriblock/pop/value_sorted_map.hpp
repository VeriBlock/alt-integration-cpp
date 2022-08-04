// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_VALUE_SORTED_MAP___
#define ALTINTEGRATION_VALUE_SORTED_MAP___

#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>

#include "assert.hpp"

namespace altintegration {

//! @private
template <typename K, typename V>
class ValueSortedMap {
 public:
  using pair_t = std::pair<const K, V>;
  using cmp_t = std::function<bool(const V&, const V&)>;
  using set_t = typename std::multiset<V, cmp_t>;
  using map_t = std::unordered_map<K, V>;

 private:
  set_t set_;
  map_t map_;

 public:
  using iterator_t = typename map_t::iterator;
  using const_iterator_t = typename map_t::const_iterator;

  ~ValueSortedMap() = default;

  explicit ValueSortedMap(cmp_t comparator) : set_(comparator) {}

  iterator_t find(const K& key) { return map_.find(key); }
  const_iterator_t find(const K& key) const { return map_.find(key); }

  auto begin() -> iterator_t { return map_.begin(); }
  auto end() -> iterator_t { return map_.end(); }
  auto begin() const -> const_iterator_t { return map_.begin(); }
  auto end() const -> const_iterator_t { return map_.end(); }

  const set_t& getSortedValues() const { return set_; }

  // implicit const map_t& cast operator
  operator const map_t&() const { return map_; }

  void erase(const K& key) {
    auto map_it = map_.find(key);
    if (map_it == map_.end()) {
      return;
    }

    auto set_it = set_.find(map_it->second);
    VBK_ASSERT(set_it != set_.end());
    set_.erase(set_it);
    map_.erase(map_it);

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());
  }

  iterator_t erase(const iterator_t& it) {
    set_.erase(set_.find(it->second));
    auto res = map_.erase(it);

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());

    return res;
  }

  void insert(const K& key, const V& value) {
    pair_t pair(key, value);

    auto map_it = map_.find(key);
    if (map_it != map_.end()) {
      // key exists
      auto set_it = set_.find(map_it->second);
      VBK_ASSERT(set_it != set_.end());
      set_.erase(set_it);

      map_it->second = value;
      set_.insert(map_it->second);
    } else {
      auto res = map_.insert(std::move(pair));
      VBK_ASSERT(res.second);
      auto it = res.first;
      set_.insert(it->second);
    }

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());
  }

  size_t size() const {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());

    return map_.size();
  }

  void clear() {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());
    set_.clear();
    map_.clear();
  }

  bool empty() const {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());

    return map_.empty();
  }
};

}  // namespace altintegration

#endif
