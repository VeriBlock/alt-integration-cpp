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

    bool operator()(const pair_t& val1, const pair_t& val2) {
      return cmp_(val1.second, val2.second);
    }

   private:
    cmp_t cmp_;
  };

 public:
  ~ValueSortedMap() = default;

  ValueSortedMap() = default;

  ValueSortedMap(const cmp_t& cmp) : set_(SetCmp(cmp)), map_() {}

  void erase(const key_t& key) {
    set_.erase(map_[key]);
    map_.erase(key);

    VBK_ASSERT_MSG(map_.size() == set_.size(),
                   "size of the map and vector incompetible map: %d, vec: %d",
                   map_.size(),
                   set_.size());
  }

  void insert(const key_t& key, const val_t& value) {
    pair_t pair(key, value);

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

 private:
  std::set<pair_t, SetCmp> set_{};
  std::unordered_map<key_t, val_t> map_{};
};

}  // namespace altintegration

#endif