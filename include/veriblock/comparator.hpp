// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_COMPARATOR_HPP
#define VERIBLOCK_POP_CPP_COMPARATOR_HPP

#include <gtest/gtest.h>

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/base_block_tree.hpp>

namespace altintegration {

struct CollectionOfPtrComparator {
  template <typename T>
  bool operator()(const std::vector<T*>& a, const std::vector<T*>& b) {
    EXPECT_EQ(a.size(), b.size());
    for (size_t i = 0, size = a.size(); i < size; i++) {
      if (a[i] == nullptr && b[i] == nullptr) {
        continue;
      }

      EXPECT_TRUE((a[i] == nullptr && b[i] != nullptr) ||
                  (b[i] == nullptr && a[i] != nullptr));
      EXPECT_EQ(*a[i], *b[i]) << "Iteration " << i;
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_map<K, std::shared_ptr<V>>& a,
                  const std::unordered_map<K, std::shared_ptr<V>>& b) {
    EXPECT_EQ(a.size(), b.size());
    for (const auto& k : a) {
      auto key = k.first;
      auto value = k.second;
      auto expectedValue = b.find(key);
      // key exists in map A but does not exist in map B
      EXPECT_NE(expectedValue, b.end());
      // pointers are equal - comparison is true
      if (expectedValue->second == value) continue;
      EXPECT_TRUE(expectedValue->second);
      EXPECT_TRUE(value);
      EXPECT_EQ(*value, *expectedValue->second);
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_multimap<K, std::shared_ptr<V>>& a,
                  const std::unordered_multimap<K, std::shared_ptr<V>>& b) {
    std::vector<V> aValues, bValues;
    std::transform(a.begin(),
                   a.end(),
                   std::back_inserter(aValues),
                   [](const std::pair<K, std::shared_ptr<V>>& item) -> V {
                     return *item.second;
                   });
    std::transform(b.begin(),
                   b.end(),
                   std::back_inserter(bValues),
                   [](const std::pair<K, std::shared_ptr<V>>& item) -> V {
                     return *item.second;
                   });

    std::sort(aValues.begin(), aValues.end());
    std::sort(bValues.begin(), bValues.end());

    EXPECT_EQ(aValues, bValues);
    return true;
  }

  template <typename T>
  bool operator()(const std::unordered_set<T*>& a,
                  const std::unordered_set<T*>& b) {
    EXPECT_EQ(a.size(), b.size());

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
    EXPECT_EQ(aHashes, bHashes);
    return true;
  }
};

template <typename Block>
bool operator==(const BaseBlockTree<Block>& a, const BaseBlockTree<Block>& b) {
  CollectionOfPtrComparator cmp{};
  EXPECT_TRUE(cmp(a.getBlocks(), b.getBlocks()));
  EXPECT_TRUE(cmp(a.getTips(), b.getTips()));
  EXPECT_EQ(a.getBestChain(), b.getBestChain());
  return true;
}

template <typename E>
bool operator==(const PopState<E>& a, const PopState<E>& b) {
  CollectionOfPtrComparator cmp;
  EXPECT_TRUE(
      cmp(a.getContainingEndorsements(), b.getContainingEndorsements()));
  EXPECT_TRUE(cmp(a.getEndorsedBy(), b.getEndorsedBy()));
  return true;
}

template <typename Block>
bool operator==(const BlockIndex<Block>& a, const BlockIndex<Block>& b) {
  EXPECT_EQ(a.getHeader(), b.getHeader());
  EXPECT_EQ(a.status, b.status);
  return true;
}

template <typename A, typename B, typename C, typename D>
bool operator==(const PopAwareForkResolutionComparator<A, B, C, D>& a,
                const PopAwareForkResolutionComparator<A, B, C, D>& b) {
  EXPECT_EQ(a.getProtectingBlockTree(), b.getProtectingBlockTree());
  return true;
}

bool operator==(const BtcBlockAddon& a, const BtcBlockAddon& b);
bool operator==(const VbkBlockAddon& a, const VbkBlockAddon& b);
bool operator==(const AltBlockAddon& a, const AltBlockAddon& b);
bool operator==(const PayloadsStorage& a, const PayloadsStorage& b);
bool operator==(const Repository& a, const Repository& b);
bool operator==(const VbkBlockTree& a, const VbkBlockTree& b);
bool operator==(const AltTree& a, const AltTree& b);
inline bool operator!=(const AltTree& a, const AltTree& b) { return !(a == b); }

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_COMPARATOR_HPP
