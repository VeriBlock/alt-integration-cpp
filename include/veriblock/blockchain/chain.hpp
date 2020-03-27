#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_

#include <cassert>
#include <map>
#include <unordered_set>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/storage/block_repository.hpp>

namespace altintegration {

/**
 * Fully in-memory chain representation.
 *
 * Every chain has exactly one block at every height.
 *
 * @note v1 implementation stores whole chain in memory. v2 will store a subset
 * of blocks in memory, to reduce memory footprint.
 * @version v1
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

  Chain() = default;

  explicit Chain(height_t startHeight) : startHeight_(startHeight) {}

  explicit Chain(height_t startHeight, index_t* tip)
      : startHeight_(startHeight) {
    setTip(tip);
  }

  height_t getStartHeight() const { return startHeight_; }

  bool contains(const index_t* index) const {
    return index != nullptr && this->operator[](index->height) == index;
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
    if (!contains(index)) {
      return nullptr;
    }
    return (*this)[index->height + 1];
  }

  height_t chainHeight() const {
    return (height_t)chain.size() + startHeight_ - 1;
  }

  size_t blocksCount() const { return chain.size(); }

  index_t* tip() const {
    return chain.empty() ? nullptr : (*this)[chainHeight()];
  }

  index_t* first() const { return chain.empty() ? nullptr : chain[0]; }

  typename storage_t::iterator begin() { return chain.begin(); }
  typename storage_t::iterator begin() const { return chain.begin(); }
  typename storage_t::iterator end() { return chain.end(); }
  typename storage_t::iterator end() const { return chain.end(); }

  void setTip(index_t* index) {
    if (index == nullptr) {
      chain.clear();
      return;
    }

    height_t innerHeight = toInnerHeight(index->height);
    chain.resize(innerHeight + 1);

    /// TODO: may stuck here forever when fed with malformed data
    while (true) {
      if (index == nullptr) break;
      if (contains(index)) break;
      if (index->height < startHeight_) break;
      innerHeight = toInnerHeight(index->height);
      chain[innerHeight] = index;
      index = index->pprev;
    }
  }

  void disconnectTip() { chain.pop_back(); }

  friend bool operator==(const Chain& a, const Chain& b) {
    // sizes may vary, so compare tips
    return a.tip() == b.tip();
  }

  const index_t* findFork(const index_t* pindex) const {
    if (pindex == nullptr || tip() == nullptr) {
      return nullptr;
    }

    auto lastHeight = chainHeight();
    if (pindex->height > lastHeight) {
      pindex = pindex->getAncestor(lastHeight);
    }
    while (pindex && !contains(pindex)) {
      pindex = pindex->pprev;
    }
    return pindex;
  }

  //! same as findFork, but returns first keystone block at or before fork point
  const index_t* findHighestKeystoneAtOrBeforeFork(const index_t* pindex,
                                                   int ki) const {
    auto* fork = findFork(pindex);
    if (!fork) {
      return nullptr;
    }
    auto keystoneHeight = highestKeystoneAtOrBefore(fork->height, ki);
    return this->operator[](keystoneHeight);
  }

  //! returns an unordered set of hashes, present in current chain.
  //! useful for small chains for further checks of "hash existence"
  std::unordered_set<hash_t> getAllHashesInChain() const {
    std::unordered_set<hash_t> ret;
    ret.reserve(chain.size() - startHeight_);

    auto* current = tip();
    while (current) {
      ret.insert(current->getHash());
      current = current->pprev;
    }

    return ret;
  }

  index_t* findBlockContainingEndorsement(
      const typename index_t::endorsement_t& e,
      const uint32_t& endorsement_settlement_interval) {
    index_t* workBlock = tip();

    for (uint32_t count = 0; count < endorsement_settlement_interval &&
                             workBlock && workBlock->height > startHeight_ &&
                             e.endorsedHash != workBlock->getHash();
         count++) {
      auto it = workBlock->containingEndorsements.find(e.id);
      if (it != workBlock->containingEndorsements.end()) {
        return workBlock;
      }
      workBlock = workBlock->pprev;
    }

    return nullptr;
  }

 private:
  height_t startHeight_ = 0;
  std::vector<index_t*> chain{};

  height_t toInnerHeight(height_t in) const {
    assert(in >= startHeight_);
    return in - startHeight_;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
