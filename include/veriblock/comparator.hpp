// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_COMPARATOR_HPP
#define VERIBLOCK_POP_CPP_COMPARATOR_HPP

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace altintegration {

struct CollectionOfPtrComparator {
  template <typename T>
  bool operator()(const std::vector<T*>& a, const std::vector<T*>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0, size = a.size(); i < size; i++) {
      if (a[i] == nullptr && b[i] == nullptr) {
        continue;
      }

      if ((a[i] == nullptr && b[i] != nullptr) ||
          (b[i] == nullptr && a[i] != nullptr)) {
        return false;
      }

      if (*a[i] != *b[i]) {
        return false;
      }
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_map<K, std::shared_ptr<V>>& a,
                  const std::unordered_map<K, std::shared_ptr<V>>& b) {
    if (a.size() != b.size()) {
      return false;
    }
    for (const auto& k : a) {
      auto key = k.first;
      auto value = k.second;
      auto expectedValue = b.find(key);
      // key exists in map A but does not exist in map B
      if (expectedValue == b.end()) {
        return false;
      }
      // pointers are equal - comparison is true
      if (expectedValue->second == value) continue;
      if (expectedValue->second == nullptr) {
        return false;
      }
      if (value == nullptr) {
        return false;
      }
      bool equal = *value == *expectedValue->second;
      if (!equal) {
        return false;
      }
    }
    return true;
  }

  template <typename T>
  bool operator()(const std::unordered_set<T*>& a,
                  const std::unordered_set<T*>& b) {
    if (a.size() != b.size()) return false;

    using H = typename T::hash_t;

    std::set<H> aHashes;
    std::transform(a.cbegin(),
                   a.cend(),
                   std::inserter(aHashes, aHashes.begin()),
                   [](const T* v) { return v->getHash(); });
    std::set<H> bHashes;
    std::transform(b.cbegin(),
                   b.cend(),
                   std::inserter(bHashes, bHashes.begin()),
                   [](const T* v) { return v->getHash(); });
    return aHashes == bHashes;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_COMPARATOR_HPP
