#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_

#include <cassert>
#include <map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository.hpp>

namespace VeriBlock {

/**
 * Full-inmemory chain representation.
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

  Chain() = default;

  explicit Chain(height_t startHeight) : startHeight_(startHeight) {}

  height_t getStartHeight() const { return startHeight_; }

  bool contains(const index_t& index) const {
    return (*this)[index.height] == &index;
  }

  index_t* operator[](height_t height) const {
    if (height < startHeight_ ||
        height > height_t(chain.size() + startHeight_)) {
      return nullptr;
    }
    return chain[height - startHeight_];
  }

  index_t* next(const index_t& index) const {
    if (contains(index)) {
      return (*this)[index.height + 1];
    } else {
      return nullptr;
    }
  }

  height_t height() const { return chain.size() - 1 + startHeight_; }

  index_t* tip() const { return chain.empty() ? nullptr : (*this)[height()]; }

  index_t* bootstrap() const { return chain.empty() ? nullptr : chain[0]; }

  void setTip(index_t* index) {
    if (index == nullptr) {
      chain.clear();
      return;
    }

    assert(index->height >= startHeight_);

    chain.resize(index->height + 1 - startHeight_);
    while (index != nullptr && operator[](index->height) != index) {
      chain[index->height - startHeight_] = index;
      index = index->pprev;
    }
  }

  friend bool operator==(const Chain& a, const Chain& b) {
    // TODO: think how to use use startHeight_
    return a.chain.size() == b.chain.size() && a.tip() == b.tip();
  }

 private:
  height_t startHeight_ = 0;
  std::vector<index_t*> chain;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
