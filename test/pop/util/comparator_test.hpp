// Copyright (c) 2019-2021 Xenios SEZC
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
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/base_block_tree.hpp>

#include "comparator_macro.hpp"

namespace altintegration {

struct TestComparator {
  TestComparator() = default;

  template <typename T>
  bool operator()(const std::vector<T*>& a,
                  const std::vector<T*>& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.size(), b.size(), suppress);
    for (size_t i = 0, size = a.size(); i < size; i++) {
      if (a[i] == nullptr && b[i] == nullptr) {
        continue;
      }

      VBK_EXPECT_TRUE((bool)(a[i] != nullptr && b[i] != nullptr), suppress);
      VBK_EXPECT_EQ(*a[i], *b[i], suppress);
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_map<K, std::unique_ptr<V>>& a,
                  const std::unordered_map<K, std::unique_ptr<V>>& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.size(), b.size(), suppress);
    for (const auto& k : a) {
      auto key = k.first;
      auto& value = k.second;
      auto expectedValue = b.find(key);
      // key exists in map A but does not exist in map B
      VBK_EXPECT_NE(expectedValue, b.end(), suppress);
      // pointers are equal - comparison is true
      VBK_EXPECT_TRUE(expectedValue->second, suppress);
      VBK_EXPECT_TRUE(value, suppress);

      VBK_EXPECT_TRUE(
          this->operator()(*value, *expectedValue->second, suppress), suppress);
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_map<K, std::set<V>>& a,
                  const std::unordered_map<K, std::set<V>>& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.size(), b.size(), suppress);
    for (const auto& k : a) {
      auto key = k.first;
      auto value = k.second;
      auto expectedSet = b.find(key);
      // key exists in map A but does not exist in map B
      VBK_EXPECT_NE(expectedSet, b.end(), suppress);

      for (const auto& el : k.second) {
        VBK_EXPECT_NE(expectedSet->second.count(el), 0, suppress);
      }
    }
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::unordered_multimap<K, std::shared_ptr<V>>& a,
                  const std::unordered_multimap<K, std::shared_ptr<V>>& b,
                  bool suppress = false) {
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

    VBK_EXPECT_EQ(aValues, bValues, suppress);
    return true;
  }

  template <typename K, typename V>
  bool operator()(const std::multimap<K, std::shared_ptr<V>>& a,
                  const std::multimap<K, std::shared_ptr<V>>& b,
                  bool suppress = false) {
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

    VBK_EXPECT_EQ(aValues, bValues, suppress);
    return true;
  }

  template <typename T>
  bool operator()(const std::unordered_set<T*>& a,
                  const std::unordered_set<T*>& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.size(), b.size(), suppress);

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
    VBK_EXPECT_EQ(aHashes, bHashes, suppress);
    return true;
  }

  template <typename T>
  bool operator()(const std::set<T*>& a,
                  const std::set<T*>& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.size(), b.size(), suppress);

    for (auto* item : a) {
      VBK_EXPECT_TRUE(item, suppress);
      VBK_EXPECT_EQ(b.count(item), 1, suppress);
    }
    return true;
  }

  template <typename Block>
  bool operator()(const BaseBlockTree<Block>& a,
                  const BaseBlockTree<Block>& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(this->operator()(a.getBlocks(), b.getBlocks(), suppress),
                    suppress);
    VBK_EXPECT_TRUE(this->operator()(a.getTips(), b.getTips(), suppress),
                    suppress);
    VBK_EXPECT_TRUE(
        this->operator()(a.getBestChain(), b.getBestChain(), suppress),
        suppress);
    return true;
  }

  template <typename Block>
  bool operator()(const BlockIndex<Block>& a,
                  const BlockIndex<Block>& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(this->operator()(a.getHeader(), b.getHeader(), suppress),
                    suppress);

    using base = typename Block::addon_t;
    const base& A = a;
    const base& B = b;
    VBK_EXPECT_TRUE(this->operator()(A, B, suppress), suppress);

    VBK_EXPECT_EQ(a.getStatus(), b.getStatus(), suppress);
    return true;
  }

  template <typename Block>
  bool operator()(const Chain<Block>& chain1,
                  const Chain<Block>& chain2,
                  bool suppress = false) {
    VBK_EXPECT_EQ(chain1.blocksCount(), chain2.blocksCount(), suppress);

    auto b1 = chain1.begin();
    auto b2 = chain2.begin();

    for (; b1 != chain1.end() && b2 != chain2.end(); ++b1, ++b2) {
      VBK_EXPECT_TRUE(this->operator()(**b1, **b2, suppress), suppress);
    }

    return true;
  }

  template <typename A, typename B, typename C, typename D>
  bool operator()(const PopAwareForkResolutionComparator<A, B, C, D>& a,
                  const PopAwareForkResolutionComparator<A, B, C, D>& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(
        this->operator()(
            a.getProtectingBlockTree(), b.getProtectingBlockTree(), suppress),
        suppress);
    return true;
  }

  bool operator()(const AltBlock& a, const AltBlock& b, bool suppress = false) {
    VBK_EXPECT_EQ(a.getHeight(), b.getHeight(), suppress);
    VBK_EXPECT_EQ(a.hash, b.hash, suppress);
    VBK_EXPECT_EQ(a.getPreviousBlock(), b.getPreviousBlock(), suppress);
    VBK_EXPECT_EQ(a.getTimestamp(), b.getTimestamp(), suppress);
    return true;
  }

  bool operator()(const BtcBlock& a, const BtcBlock& b, bool suppress = false) {
    VBK_EXPECT_EQ(a.getVersion(), b.getVersion(), suppress);
    VBK_EXPECT_EQ(a.getTimestamp(), b.getTimestamp(), suppress);
    VBK_EXPECT_EQ(a.getNonce(), b.getNonce(), suppress);
    VBK_EXPECT_EQ(a.getMerkleRoot(), b.getMerkleRoot(), suppress);
    VBK_EXPECT_EQ(a.getPreviousBlock(), b.getPreviousBlock(), suppress);
    return true;
  }

  bool operator()(const VbkBlock& a, const VbkBlock& b, bool suppress = false) {
    VBK_EXPECT_EQ(a.getVersion(), b.getVersion(), suppress);
    VBK_EXPECT_EQ(a.getTimestamp(), b.getTimestamp(), suppress);
    VBK_EXPECT_EQ(a.getNonce(), b.getNonce(), suppress);
    VBK_EXPECT_EQ(a.getPreviousBlock(), b.getPreviousBlock(), suppress);
    VBK_EXPECT_EQ(a.getPreviousKeystone(), b.getPreviousKeystone(), suppress);
    VBK_EXPECT_EQ(
        a.getSecondPreviousKeystone(), b.getSecondPreviousKeystone(), suppress);
    VBK_EXPECT_EQ(a.getHeight(), b.getHeight(), suppress);
    return true;
  }

  bool operator()(const BtcBlockAddon& a,
                  const BtcBlockAddon& b,
                  bool suppress = false) {
    // comparing reference counts does not seem like a good idea
    // as the only situation where they would be different is
    // comparing blocks across different trees eg mock miner vs
    // the test tree and in this situation the references and counts
    // are likely to differ

    // EXPECT_EQ(a.getRefs(), b.getRefs());
    VBK_EXPECT_EQ(a.chainWork, b.chainWork, suppress);
    return true;
  }

  bool operator()(const VbkBlockAddon& a,
                  const VbkBlockAddon& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(a.chainWork, b.chainWork, suppress);
    VBK_EXPECT_EQ(a.refCount(), b.refCount(), suppress);
    VBK_EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>(), suppress);

    using base = PopState<VbkEndorsement>;
    const base& A = a;
    const base& B = b;
    VBK_EXPECT_TRUE(this->operator()(A, B, suppress), suppress);
    return true;
  }

  template <typename E>
  bool operator()(const PopState<E>& a,
                  const PopState<E>& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(this->operator()(a.getContainingEndorsements(),
                                     b.getContainingEndorsements(),
                                     suppress),
                    suppress);
    VBK_EXPECT_TRUE(
        this->operator()(a.getEndorsedBy(), b.getEndorsedBy(), suppress),
        suppress);
    return true;
  }

  bool operator()(const AltBlockAddon& a,
                  const AltBlockAddon& b,
                  bool suppress = false) {
    VBK_EXPECT_EQ(
        a.getPayloadIds<VbkBlock>(), b.getPayloadIds<VbkBlock>(), suppress);
    VBK_EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>(), suppress);
    VBK_EXPECT_EQ(a.getPayloadIds<ATV>(), b.getPayloadIds<ATV>(), suppress);

    using base = PopState<AltEndorsement>;
    const base& A = a;
    const base& B = b;
    VBK_EXPECT_TRUE(this->operator()(A, B, suppress), suppress);
    return true;
  }

  bool operator()(const VbkBlockTree& a,
                  const VbkBlockTree& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(
        this->operator()(a.getComparator(), b.getComparator(), suppress),
        suppress);
    using base = VbkBlockTree::base;
    const base& A = a;
    const base& B = b;
    VBK_EXPECT_TRUE(this->operator()(A, B, suppress), suppress);
    return true;
  }

  bool operator()(const AltBlockTree& a,
                  const AltBlockTree& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(
        this->operator()(a.getComparator(), b.getComparator(), suppress),
        suppress);
    using base = AltBlockTree::base;
    const base& A = a;
    const base& B = b;
    VBK_EXPECT_TRUE(this->operator()(A, B, suppress), suppress);
    VBK_EXPECT_TRUE(
        this->operator()(a.getPayloadsIndex(), b.getPayloadsIndex(), suppress),
        suppress);
    // do not compare payloads providers here
    return true;
  }

  bool operator()(const PayloadsIndex& a,
                  const PayloadsIndex& b,
                  bool suppress = false) {
    VBK_EXPECT_TRUE(
        this->operator()(a.getPayloadsInAlt(), b.getPayloadsInAlt(), suppress),
        suppress);
    VBK_EXPECT_TRUE(
        this->operator()(a.getPayloadsInVbk(), b.getPayloadsInVbk(), suppress),
        suppress);

    return true;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_COMPARATOR_HPP
