// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FR_CACHE_HPP
#define VERIBLOCK_POP_CPP_FR_CACHE_HPP

namespace altintegration {

enum class PopComparisonResult {
  // cache miss
  unknown,

  // chains are equal
  equal,  // result = 0

  // chain is known to be better
  A_better,  // result = 1
  B_better,  // result = -1

  // chain is thought to be better, because its previous block is better in this
  // comparison. If A>B, then A+k > B.
  A_transitively_better,
  B_transitively_better
};

/**
 * Cache for Fork Resolution outcome.
 *
 * If chains A and B compared, outcome is stored in this cache.
 * If new payloads are added to either of chains, we evict this comparison from
 * cache.
 * If A is better than B, comparison is stored in cache; and A is active, and we
 * do POP FR of block A+k and B, A+k is transitively better (cache hit).
 * If A is better than B, comparison is stored in cache; and A is active, and we
 * compare block A and B+k, this comparison is a cache miss.
 * If A is better than B, and B is on a fork, A is better than all blocks in a B
 *
 * @tparam BlockIndexT
 */
template <typename BlockIndexT>
struct FrCache {
  using index_t = BlockIndexT;
  using block_t = typename index_t::block_t;
  using hash_t = typename block_t::hash_t;

  using map = std::unordered_map<const index_t*, std::set<const index_t*>>;

 private:
  // set of blocks where KEY is known to be better than any block in a VALUE set
  // if A=key, B=one of values in a set; return 1
  // if B=key, A=one of values in a set; return -1
  std::unordered_map<const index_t*, std::set<const index_t*>> winners_;
  // set of blocks where KEY is known to be equal to any block in a VALUE set
  // if A=key, B=one of values in a set; return 0
  // if B=key, A=one of values in a set; return 0
  std::unordered_map<const index_t*, std::set<const index_t*>> equal_;

  double misses_ = 0;
  double hitsEqual_ = 0;
  double hitsAbetter_ = 0;
  double hitsBbetter_ = 0;
  double hitsAtrbetter_ = 0;
  double hitsBtrbetter_ = 0;

  // minimal height stored in a cache
  int minHeight_ = std::numeric_limits<int>::max();

 public:
  double misses() const { return misses_; }
  double hitsEqual() const { return hitsEqual_; }
  double hitsAbetter() const { return hitsAbetter_; }
  double hitsBbetter() const { return hitsBbetter_; }
  double hitsAtransitiveBetter() const { return hitsAtrbetter_; }
  double hitsBtransitiveBetter() const { return hitsBtrbetter_; }

  double hitRate() const {
    double hits = hitsEqual_ + hitsAbetter_ + hitsBbetter_ + hitsAtrbetter_ +
                  hitsBtrbetter_;
    double total = hits + misses_;

    // prevent division by 0
    if (total == 0) {
      return 0;
    }

    return hits / total;
  }

  // block A is known to be updated with new payloads, evict this block from
  // cache
  void evict(const index_t& a) {
    winners_.erase(&a);
    equal_.erase(&a);

    for (auto& p : winners_) {
      auto& set = p.second;
      set.erase(&a);
    }

    for (auto& p : equal_) {
      auto& set = p.second;
      set.erase(&a);
    }
  }

  int store(const index_t& a, const index_t& b, int result) {
    if (result > 0) {
      // a > b
      winners_[&a].insert(&b);
    } else if (result < 0) {
      // b > a
      winners_[&b].insert(&a);
    } else {
      // a == b -> b == a
      equal_[&a].insert(&b);
      equal_[&b].insert(&a);
    }

    minHeight_ = std::min({a.height, b.height, minHeight_});
    return result;
  }

  PopComparisonResult compare(const index_t& a, const index_t& b) {
    PopComparisonResult r = compareImpl(a, b);
    switch (r) {
      case PopComparisonResult::unknown:
        VBK_LOG_DEBUG("Cache miss %s vs %s", a.toShortPrettyString(), b.toShortPrettyString());
        misses_++;
        break;
      case PopComparisonResult::equal:
        VBK_LOG_DEBUG("Cache hit %s == %s", a.toShortPrettyString(), b.toShortPrettyString());
        hitsEqual_++;
        break;
      case PopComparisonResult::A_better:
        VBK_LOG_DEBUG("Cache hit %s > %s", a.toShortPrettyString(), b.toShortPrettyString());
        hitsAbetter_++;
        break;
      case PopComparisonResult::B_better:
        VBK_LOG_DEBUG("Cache hit %s < %s", a.toShortPrettyString(), b.toShortPrettyString());
        hitsBbetter_++;
        break;
      case PopComparisonResult::A_transitively_better:
        VBK_LOG_DEBUG("Cache hit (transitive) %s > %s", a.toShortPrettyString(), b.toShortPrettyString());
        hitsAtrbetter_++;
        break;
      case PopComparisonResult::B_transitively_better:
        VBK_LOG_DEBUG("Cache hit (transitive) %s < %s", a.toShortPrettyString(), b.toShortPrettyString());
        hitsBtrbetter_++;
        break;
    }

    return r;
  }

 private:
  // returns true on cache hit, false otherwise
  PopComparisonResult compareImpl(const index_t& a, const index_t& b) {
    if (test(winners_, &a, &b)) {
      // A is known to be better than B
      return PopComparisonResult::A_better;
    }

    if (test(winners_, &b, &a)) {
      // B is known to be better than A
      return PopComparisonResult::B_better;
    }

    if (test(equal_, &a, &b)) {
      // A is known to be equal to B
      return PopComparisonResult::equal;
    }

    // optimization: equality is commutative, so test only once (if(A==B), and
    // not B==A)

    const Chain<index_t> achain(minHeight_, &a);
    auto* aprev = findCachedPrevBlock(achain, winners_);
    if (aprev != nullptr) {
      // is previous of A known to be better than B?
      if (test(winners_, aprev, &b)) {
        // A is transitively better than B
        return PopComparisonResult::A_transitively_better;
      }
    }

    const Chain<index_t> bchain(minHeight_, &b);
    auto* bprev = findCachedPrevBlock(bchain, winners_);
    if (bprev != nullptr) {
      // is previous of B known to be better than A?
      if (test(winners_, bprev, &a)) {
        // B is transitively better than A
        return PopComparisonResult::B_transitively_better;
      }
    }

    // cache miss
    return PopComparisonResult::unknown;
  }

  bool test(map& m, const index_t* a, const index_t* b) {
    auto ita = m.find(a);
    if (ita != m.end()) {
      // A has been found in cache
      auto& set = ita->second;
      return set.count(b) > 0;
    }

    // cache miss
    return false;
  }

  // returns nullptr if no prev block cached
  const index_t* findCachedPrevBlock(const Chain<index_t>& chain, map& m) {
    for (auto* i : reverse_iterate(chain)) {
      if (m.count(i) > 0) {
        return i;
      }
    }

    // no previous block found
    return nullptr;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FR_CACHE_HPP
