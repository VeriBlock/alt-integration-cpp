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

  iterator_t find(const key_t& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      return set_.find(*it);
    }
    return set_.end();
  }

  iterator_t& begin() { return set_.begin(); }

  iterator_t end() { return set_.end(); }

  void erase(const key_t& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      set_.erase(*it);
      map_.erase(it);
    }

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());
  }

  void insert(const key_t& key, const val_t& value) {
    pair_t pair(key, value);

    auto it = map_.find(key);


    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());
  }

  size_t size() const {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());

    return map_.size();
  }

  void clear() {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());
    set_.clear();
    map_.clear();
  }

  bool empty() const {
    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());

    return map_.empty();
  }

 private:
  std::set<pair_t, SetCmp> set_{};
  std::unordered_map<key_t, val_t> map_{};
};

}  // namespace altintegration

#endif