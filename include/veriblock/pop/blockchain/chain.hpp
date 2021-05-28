// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_

#include <map>
#include <unordered_set>
#include <veriblock/pop/keystone_util.hpp>

#include "block_index.hpp"

namespace altintegration {

/**
 * Fully in-memory chain representation.
 *
 * Every chain has exactly one block at every height.
 *
 * @tparam Block
 */
template <typename BlockIndexT>
struct Chain {
  using index_t = BlockIndexT;
  using block_t = typename index_t::block_t;
  using hash_t = typename index_t::hash_t;
  using height_t = typename block_t::height_t;
  using storage_t = std::vector<index_t*>;

  using iterator_t = typename storage_t::iterator;
  using const_iterator_t = typename storage_t::const_iterator;
  using reverse_iterator_t = typename storage_t::reverse_iterator;
  using const_reverse_iterator_t = typename storage_t::const_reverse_iterator;

  Chain() = default;

  explicit Chain(height_t startHeight) : startHeight_(startHeight) {}

  explicit Chain(height_t startHeight, index_t* tip)
      : startHeight_(startHeight) {
    setTip(tip);
  }

  // for compatibility
  height_t getStartHeight() const { return firstHeight(); }
  height_t firstHeight() const { return startHeight_; }

  bool contains(const index_t* index) const {
    return index != nullptr && this->operator[](index->getHeight()) == index;
  }

  index_t* operator[](height_t height) const {
    if (height < startHeight_) return nullptr;

    height_t innerHeight = toInnerHeight(height);
    if (innerHeight >= height_t(chain.size())) {
      return nullptr;
    }
    return chain[innerHeight];
  }

  index_t* next(const index_t* index) const {
    return !contains(index) ? nullptr : (*this)[index->getHeight() + 1];
  }

  height_t chainHeight() const {
    return (height_t)chain.size() + startHeight_ - 1;
  }

  bool empty() const { return chain.empty(); }

  size_t size() const { return chain.size(); }
  // for compatibility
  size_t blocksCount() const { return size(); }

  index_t* tip() const {
    return chain.empty() ? nullptr : (*this)[chainHeight()];
  }

  index_t* first() const { return chain.empty() ? nullptr : chain[0]; }

  reverse_iterator_t rbegin() { return chain.rbegin(); }
  const_reverse_iterator_t rbegin() const { return chain.rbegin(); }
  reverse_iterator_t rend() { return chain.rend(); }
  const_reverse_iterator_t rend() const { return chain.rend(); }

  iterator_t begin() { return chain.begin(); }
  const_iterator_t begin() const { return chain.begin(); }
  iterator_t end() { return chain.end(); }
  const_iterator_t end() const { return chain.end(); }

  void appendRoot(index_t* index) {
    // set a new bootstrap block which should be connected to the current one
    VBK_ASSERT_MSG(index == first()->pprev,
                   "new bootstrap block does not connects to the current one");
    startHeight_ = index->getHeight();
    chain.insert(chain.begin(), index);
  }

  void setTip(index_t* index) {
    if (index == nullptr || index->getHeight() < startHeight_) {
      chain.clear();
      return;
    }

    height_t innerHeight = toInnerHeight(index->getHeight());
    chain.resize(innerHeight + 1);

    while (index != nullptr && !contains(index) &&
           index->getHeight() >= startHeight_) {
      innerHeight = toInnerHeight(index->getHeight());
      chain[innerHeight] = index;
      index = index->getPrev();
    }
  }

  void disconnectTip() { chain.pop_back(); }

  friend bool operator==(const Chain& a, const Chain& b) {
    // sizes may vary, so compare tips
    if (a.tip() == nullptr && b.tip() == nullptr) return true;
    if (a.tip() == nullptr) return false;
    if (b.tip() == nullptr) return false;
    return a.tip()->getHash() == b.tip()->getHash();
  }

  friend bool operator!=(const Chain& a, const Chain& b) { return !(a == b); }

 private:
  height_t startHeight_ = 0;
  std::vector<index_t*> chain{};

  height_t toInnerHeight(height_t in) const {
    VBK_ASSERT(in >= startHeight_);
    return in - startHeight_;
  }
};

//! Find fork between `chain` and `pindex`.
//! @returns nullptr if fork can not be found
template <typename C>
typename C::index_t* findFork(const C& chain,
                              const typename C::index_t* pindex) {
  if (pindex == nullptr || chain.tip() == nullptr) {
    return nullptr;
  }

  auto lastHeight = chain.chainHeight();
  if (pindex->getHeight() > lastHeight) {
    pindex = pindex->getAncestor(lastHeight);
  }
  while (pindex && !chain.contains(pindex)) {
    pindex = pindex->getPrev();
  }
  return const_cast<typename C::index_t*>(pindex);
}

//! returns an unordered set of hashes, present in current chain.
//! useful for small chains for further checks of "hash existence"
template <typename C>
std::unordered_set<typename C::hash_t> getAllHashesInChain(const C& chain) {
  std::unordered_set<typename C::hash_t> ret;
  ret.reserve(chain.size());

  for (auto* block : chain) {
    ret.emplace(block->getHash());
  }

  return ret;
}

//! @private
template <typename index_t>
const index_t* findBlockContainingEndorsement(
    const Chain<index_t>& chain,
    const index_t* workBlock,
    const typename index_t::endorsement_t::id_t& id,
    const uint32_t& window) {
  for (uint32_t count = 0; count < window && workBlock &&
                           workBlock->getHeight() >= chain.getStartHeight();
       count++) {
    if (workBlock->getContainingEndorsements().count(id)) {
      return workBlock;
    }
    workBlock = workBlock->pprev;
  }

  return nullptr;
}

//! @private
template <typename index_t>
inline const index_t* findBlockContainingEndorsement(
    const Chain<index_t>& chain,
    const typename index_t::endorsement_t& e,
    const uint32_t& endorsement_settlement_interval) {
  return findBlockContainingEndorsement<index_t>(
      chain, chain.tip(), e.id, endorsement_settlement_interval);
}

//! @private
template <typename T>
void PrintTo(const Chain<T>& c, std::ostream* os) {
  auto* tip = c.tip();
  if (!tip) {
    *os << fmt::format("Chain(height={} tip=empty)", c.getStartHeight());
    return;
  }

  *os << "Chain(height=" << c.getStartHeight() << "\n";
  while (tip != nullptr) {
    *os << tip->toPrettyString() << "\n";
    tip = tip->pprev;
  }
  *os << ")\n";
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
