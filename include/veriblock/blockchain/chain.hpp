#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_

#include <cassert>
#include <map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository.hpp>

namespace VeriBlock {

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
template <typename Block>
struct Chain {
  using block_t = Block;
  using index_t = BlockIndex<block_t>;
  using height_t = typename Block::height_t;
  using storage_t = std::vector<index_t*>;

  Chain() = default;

  explicit Chain(height_t startHeight) : startHeight_(startHeight) {}

  explicit Chain(height_t startHeight, index_t* tip)
      : startHeight_(startHeight) {
    setTip(tip);
  }

  height_t getStartHeight() const { return startHeight_; }

  bool contains(const index_t* index) const {
    return index != nullptr && (*this)[index->height] == index;
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

    if(tip() == nullptr || (tip() != nullptr && tip() == index->pprev)) {
      chain.push_back(index);
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
    // TODO: think how to use use startHeight_
    return a.chain.size() == b.chain.size() && a.tip() == b.tip();
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

 private:
  height_t startHeight_ = 0;
  std::vector<index_t*> chain{};

  height_t toInnerHeight(height_t in) const {
    assert(in >= startHeight_);
    return in - startHeight_;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
