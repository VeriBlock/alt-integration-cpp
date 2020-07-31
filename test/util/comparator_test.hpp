// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_COMPARATOR_TEST_HPP
#define VERIBLOCK_POP_CPP_COMPARATOR_TEST_HPP

#include <gtest/gtest.h>

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/base_block_tree.hpp>

namespace altintegration {

struct TestComparator {
  template <typename T>
  bool operator()(const std::vector<T*>& a, const std::vector<T*>& b) {
    EXPECT_EQ(a.size(), b.size());
    for (size_t i = 0, size = a.size(); i < size; i++) {
      if (a[i] == nullptr && b[i] == nullptr) {
        continue;
      }

      EXPECT_TRUE((a[i] == nullptr && b[i] != nullptr) ||
                  (b[i] == nullptr && a[i] != nullptr));

      EXPECT_TRUE(this->operator()(*a[i], *b[i])) << "Iteration " << i;
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
      EXPECT_TRUE(expectedValue->second);
      EXPECT_TRUE(value);

      EXPECT_TRUE(this->operator()(*value, *expectedValue->second));
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::map<K, std::set<V>>& a,
                  const std::map<K, std::set<V>>& b) {
    EXPECT_EQ(a.size(), b.size());
    for (const auto& k : a) {
      auto key = k.first;
      auto value = k.second;
      auto expectedSet = b.find(key);
      // key exists in map A but does not exist in map B
      ASSERT_NE(expectedSet, b.end());

      for (const auto& el : k.second) {
        EXPECT_EQ(expectedSet->second, el);
      }
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

  template <typename Block>
  bool operator()(const BaseBlockTree<Block>& a,
                  const BaseBlockTree<Block>& b) {
    EXPECT_TRUE(this->operator()(a.getBlocks(), b.getBlocks()));
    EXPECT_TRUE(this->(a.getTips(), b.getTips()));
    EXPECT_TRUE(this->operator()(a.getBestChain(), b.getBestChain()));
    return true;
  }

  template <typename E>
  bool operator()(const PopState<E>& a, const PopState<E>& b) {
    EXPECT_TRUE(this->operator()(a.getContainingEndorsements(),
                                 b.getContainingEndorsements()));
    EXPECT_TRUE(this->operator()(a.getEndorsedBy(), b.getEndorsedBy()));
    return true;
  }

  template <typename Block>
  bool operator()(const BlockIndex<Block>& a, const BlockIndex<Block>& b) {
    EXPECT_TRUE(this->operator()(a.getHeader(), b.getHeader()));
    EXPECT_EQ(a.status, b.status);
    return true;
  }

  template <typename A, typename B, typename C, typename D>
  bool operator()(const PopAwareForkResolutionComparator<A, B, C, D>& a,
                  const PopAwareForkResolutionComparator<A, B, C, D>& b) {
    EXPECT_TRUE(this->operator()(a.getProtectingBlockTree(),
                                 b.getProtectingBlockTree()));
    return true;
  }

  bool operator()(const BtcBlockAddon& a, const BtcBlockAddon& b) {
    // comparing reference counts does not seem like a good idea
    // as the only situation where they would be different is
    // comparing blocks across different trees eg mock miner vs
    // the test tree and in this situation the references and counts
    // are likely to differ
    EXPECT_EQ(a.getRefs(), b.getRefs());
    EXPECT_EQ(a.chainWork, b.chainWork);
    return true;
  }

  bool operator()(const VbkBlockAddon& a, const VbkBlockAddon& b) {
    EXPECT_EQ(a.chainWork, b.chainWork);
    EXPECT_EQ(a.refCount(), b.refCount());
    EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>());

    using base = PopState<VbkEndorsement>;
    const base& A = a;
    const base& B = b;
    EXPECT_EQ(A, B);
    return true;
  }

  bool operator()(const AltBlockAddon& a, const AltBlockAddon& b) {
    EXPECT_EQ(a.getPayloadIds<VbkBlock>(), b.getPayloadIds<VbkBlock>());
    EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>());
    EXPECT_EQ(a.getPayloadIds<ATV>(), b.getPayloadIds<ATV>());

    using base = PopState<AltEndorsement>;
    const base& A = a;
    const base& B = b;
    EXPECT_EQ(A, B);
    return true;
  }

  bool operator()(const VbkBlockTree& a, const VbkBlockTree& b) {
    EXPECT_TRUE(this->operator()(a.getComparator(), b.getComparator()));
    using base = VbkBlockTree::base;
    const base& A = a;
    const base& B = b;
    EXPECT_TRUE(this->operator()(A, B));
    return true;
  }

  bool operator()(const AltTree& a, const AltTree& b) {
    EXPECT_EQ(a.getComparator(), b.getComparator());
    using base = AltTree::base;
    const base& A = a;
    const base& B = b;
    EXPECT_TRUE(this->operator()(A, B));
    EXPECT_TRUE(this->operator()(a.getStorage(), b.getStorage()));
    return true;
  }

  bool operator()(const PayloadsStorage& a, const PayloadsStorage& b) {
    EXPECT_TRUE(this->operator()(a.getRepo(), b.getRepo()));

    this->operator()(a.getPayloadsInAlt(), b.getPayloadsInAlt());
    this->operator()(a.getPayloadsInVbk(), b.getPayloadsInVbk());

    EXPECT_TRUE(this->operator()(a.getPayloadsInAlt(), b.getPayloadsInAlt()));
    EXPECT_TRUE(this->operator()(a.getPayloadsInVbk(), b.getPayloadsInVbk()));

    EXPECT_EQ(a.getValidity(), b.getValidity());
    return true;
  }

  bool operator()(const Repository& a, const Repository& b) {
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> mA;
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> mB;

    readRepositoryInto(mA, a);
    readRepositoryInto(mB, b);

    EXPECT_EQ(mA, mB);
    return true;
  }

 private:
  void readRepositoryInto(
      std::map<std::vector<uint8_t>, std::vector<uint8_t>>& m,
      const Repository& r) {
    auto cursor = r.newCursor();
    cursor->seekToFirst();
    while (cursor->isValid()) {
      m.insert({cursor->key(), cursor->value()});
      cursor->next();
    }
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_COMPARATOR_HPP
