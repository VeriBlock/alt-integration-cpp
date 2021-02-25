// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_VALUE_SORTED_MAP___
#define ALTINTEGRATION_VALUE_SORTED_MAP___

#include <algorithm>
#include <set>
#include <unordered_map>

#include "veriblock/assert.hpp"

namespace altintegration {

template <typename key_t, typename val_t, typename cmp_t>
struct ValueSortedMap {
  using pair_t = std::pair<key_t, val_t>;

 private:
  struct SetCmp {
    ~SetCmp() = default;

    SetCmp() = default;

    SetCmp(const cmp_t& cmp) : cmp_(cmp) {}

    bool operator()(const pair_t& val1, const pair_t& val2) const {
      return cmp_(val1.second, val2.second);
    }

   private:
    cmp_t cmp_{};
  };

 public:
  using iterator_t = typename std::set<pair_t, SetCmp>::iterator;

  ~ValueSortedMap() = default;

  ValueSortedMap() = default;

  ValueSortedMap(const cmp_t& cmp) : set_(SetCmp(cmp)), map_() {}

  iterator_t find(const key_t& key) const {
    auto it = map_.find(key);
    return it != map_.end() ? set_.find(*it) : set_.end();
  }

  iterator_t begin() const { return set_.begin(); }

  iterator_t end() const { return set_.end(); }

  void erase(const key_t& key) {
    auto map_it = map_.find(key);

    if (map_it != map_.end()) {
      auto set_it = set_.find(*map_it);
      VBK_ASSERT(set_it != set_.end());
      set_.erase(set_it);
      map_.erase(map_it);
    }

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());
  }

  iterator_t erase(const iterator_t& it) {
    map_.erase(it->first);
    auto res = set_.erase(it);

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of map and set are different map: %d, set: %d",
                   map_.size(),
                   set_.size());

    return res;
  }

  void insert(const key_t& key, const val_t& value) {
    pair_t pair(key, value);

    auto map_it = map_.find(key);
    if (map_it != map_.end()) {
      auto set_it = set_.find(*map_it);
      VBK_ASSERT(set_it != set_.end());
      set_.erase(set_it);

      map_it->second = value;
      set_.insert(pair);
    } else {
      map_.insert(pair);
      set_.insert(pair);
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

 private:
  std::multiset<pair_t, SetCmp> set_{};
  std::unordered_map<key_t, val_t> map_{};
};

}  // namespace altintegration

#endif