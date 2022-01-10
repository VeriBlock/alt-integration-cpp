// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_SMALL_LRU_CACHE_HPP
#define VERIBLOCK_POP_CPP_SMALL_LRU_CACHE_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <veriblock/pop/time.hpp>

namespace altintegration {
namespace cache {

//! time-based LFRU cache.
//!
//! if all cache items are newer than TimeWindow min, evict less frequently used
//! element. otherwise, evict oldest item, with lowest frequency.
//!
//! has O(Size) complexity, thus works best for caches with small number of keys
template <typename Key,
          typename Value,
          size_t Size,
          size_t TimeWindow = 10 * 60,  // 10 min
          typename = typename std::enable_if<(Size <= 50)>::type>
struct SmallLFRUCache {
  //! @private
  struct Item {
    size_t frequency = 0;
    size_t lastAccessed = 0;
    Key key;
    std::shared_ptr<Value> value = nullptr;
  };

  std::shared_ptr<Value> getOrDefault(
      Key key, std::function<std::shared_ptr<Value>()> factory) {
    std::shared_ptr<Value> value;
    if (!get(key, value)) {
      value = factory();
      insert(key, value);
    }

    return value;
  }

  void clear() {
    size_ = 0;
    for (auto& i : container_) {
      i.value = nullptr;
    }
  }

  void insert(Key key, std::shared_ptr<Value> value) {
    auto current = currentTimestamp4();

    if (size_ < Size) {
      // cache can accept new elements
      auto& it = container_[size_++];
      it.frequency = 0;
      it.key = key;
      it.value = std::move(value);
      it.lastAccessed = current;
      return;
    }

    // it's time to evict someone
    auto begin = container_.begin();
    auto end = container_.end();
    // iterator to least frequency item
    auto min = begin;
    // iterator to least recently used item
    auto leastRecent = begin;
    for (auto it = begin; it != end; ++it) {
      if (it->frequency < min->frequency) {
        min = it;
      }

      if (it->lastAccessed < leastRecent->lastAccessed ||
          (it->lastAccessed == leastRecent->lastAccessed &&
           it->frequency < leastRecent->frequency)) {
        leastRecent = it;
      }
    }

    // by default, use LFU policy and evict element with min frequency
    auto evict = min;
    if (leastRecent->lastAccessed <= current - TimeWindow) {
      // if least recently element is not used in TimeWindow sec, evict it
      evict = leastRecent;
    }

    // do evict
    evict->key = key;
    evict->frequency = 0;
    evict->value = std::move(value);
    evict->lastAccessed = current;
  }

  bool get(const Key& key, std::shared_ptr<Value>& out) {
    // search for key
    auto begin = container_.begin();
    auto end = container_.begin() + size_;
    auto it = std::find_if(
        begin, end, [&key](const Item& i) { return i.key == key; });
    if (it == end) {
      // key not found
      return false;
    }

    it->frequency++;
    it->lastAccessed = currentTimestamp4();

    // return result
    out = it->value;

    return true;
  }

 private:
  std::array<Item, Size> container_;

  // number of elements in a cache
  size_t size_ = 0;
};

}  // namespace cache
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_SMALL_LRU_CACHE_HPP
