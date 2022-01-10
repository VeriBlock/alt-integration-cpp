// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_SLICE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_SLICE_HPP_

#include <veriblock/pop/blockchain/block_index.hpp>

namespace altintegration {

/**
 * Non-owning partial chain view similar to std::span.
 *
 * API-compatible with Chain.
 * @tparam BlockIndex
 */
template <typename BlockIndexT>
struct ChainSlice {
  using index_t = BlockIndexT;
  using block_t = typename index_t::block_t;
  using hash_t = typename index_t::hash_t;
  using height_t = typename block_t::height_t;
  using chain_t = Chain<index_t>;

  using const_iterator_t = typename chain_t::const_iterator_t;
  using const_reverse_iterator_t = typename chain_t::const_reverse_iterator_t;

  ChainSlice(const chain_t& chain) : ChainSlice(chain, chain.firstHeight()) {}

  explicit ChainSlice(const chain_t& chain, height_t firstHeight)
      : ChainSlice(chain,
                   firstHeight,
                   chain.firstHeight() + chain.blocksCount() - firstHeight) {}

  explicit ChainSlice(const chain_t& chain, height_t firstHeight, size_t size)
      : chain_(chain), firstHeight_(firstHeight), size_(size) {
    VBK_ASSERT(firstHeight >= chain.firstHeight());
    VBK_ASSERT(firstHeight + size <= chain.firstHeight() + chain.blocksCount());
  }

  // for compatibility
  height_t getStartHeight() const { return firstHeight(); }

  bool contains(const index_t* index) const {
    return index != nullptr && this->operator[](index->getHeight()) == index;
  }

  index_t* operator[](height_t height) const {
    return height < firstHeight() || height > tipHeight() ? nullptr
                                                          : chain_[height];
  }

  index_t* next(const index_t* index) const {
    return !contains(index) ? nullptr : (*this)[index->getHeight() + 1];
  }

  height_t firstHeight() const { return firstHeight_; }

  height_t tipHeight() const { return (int)(firstHeight() + size()) - 1; }

  // for compatibility
  height_t chainHeight() const { return tip()->getHeight(); }

  bool empty() const { return size() == 0; }

  size_t size() const { return size_; }
  // for compatibility
  size_t blocksCount() const { return size(); }

  index_t* tip() const { return empty() ? nullptr : chain_[tipHeight()]; }
  index_t* first() const { return empty() ? nullptr : chain_[firstHeight()]; }

  size_t firstOffset() const { return firstHeight() - chain_.firstHeight(); }
  size_t tipOffset() const { return chain_.chainHeight() - tipHeight(); }

  const_reverse_iterator_t rbegin() const {
    return chain_.rbegin() + tipOffset();
  }
  const_reverse_iterator_t rend() const {
    return chain_.rend() - firstOffset();
  }

  const_iterator_t begin() const { return chain_.begin() + firstOffset(); }
  const_iterator_t end() const { return chain_.end() - tipOffset(); }

  friend bool operator==(const ChainSlice& a, const ChainSlice& b) {
    // one of the slices is empty, but the other is not
    if (a.empty() != b.empty()) return false;
    // both slices are empty
    if (a.empty()) return true;
    return a.tip()->getHash() == b.tip()->getHash();
  }

  friend bool operator!=(const ChainSlice& a, const ChainSlice& b) {
    return !(a == b);
  }

 private:
  const chain_t& chain_;
  const height_t firstHeight_;
  const size_t size_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_SLICE_HPP_
