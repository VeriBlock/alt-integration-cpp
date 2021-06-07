// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TEMP_BLOCK_TREE_HPP
#define ALTINTEGRATION_TEMP_BLOCK_TREE_HPP

#include <memory>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

//! @private
template <typename StableBlockTree>
struct TempBlockTree {
  //! @private
  template <typename BlockIndexT>
  struct TempBlockIndex : public BlockIndexT {
    using base = BlockIndexT;
    using base::base;

    ~TempBlockIndex() {
      // if alttree is destroyed before temp block tree, then
      // `pprev` may point to deallocated block. to solve this, override
      // destructor of temp block index, as we don't care in which order temp
      // blocks are destroyed.
      this->pprev = nullptr;
      this->pnext.clear();
    }
  };

  using block_tree_t = StableBlockTree;
  using block_t = typename block_tree_t::block_t;
  using index_t = typename block_tree_t::index_t;
  using wrapped_index_t = TempBlockIndex<index_t>;
  using prev_block_hash_t = typename StableBlockTree::prev_block_hash_t;
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::unique_ptr<wrapped_index_t>>;

  TempBlockTree(const block_tree_t& tree) : tree_(&tree) {}

  virtual ~TempBlockTree() = default;

  // non-copyable
  TempBlockTree(const TempBlockTree&) = delete;
  TempBlockTree& operator=(const TempBlockTree&) = delete;

  // movable
  TempBlockTree(TempBlockTree&&) = default;
  TempBlockTree& operator=(TempBlockTree&&) = default;

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, typename block_t::hash_t>::value ||
                std::is_same<T, typename block_t::prev_hash_t>::value>::type>
  const index_t* getTempBlockIndex(const T& hash) const {
    auto shortHash = tree_->makePrevHash(hash);
    auto it = temp_blocks_.find(shortHash);
    return it == temp_blocks_.end() ? nullptr : it->second.get();
  }

  //! @overload
  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, typename block_t::hash_t>::value ||
                std::is_same<T, typename block_t::prev_hash_t>::value>::type>
  index_t* getTempBlockIndex(const T& hash) {
    const auto& t = as_const(*this);
    return const_cast<index_t*>(t.getTempBlockIndex(hash));
  }

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, typename block_t::hash_t>::value ||
                std::is_same<T, typename block_t::prev_hash_t>::value>::type>
  const index_t* getBlockIndex(const T& hash) const {
    auto* index = getTempBlockIndex(hash);
    return index == nullptr ? tree_->getBlockIndex(hash) : index;
  }

  //! @overload
  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, typename block_t::hash_t>::value ||
                std::is_same<T, typename block_t::prev_hash_t>::value>::type>
  index_t* getBlockIndex(const T& hash) {
    const auto& t = as_const(*this);
    return const_cast<index_t*>(t.getBlockIndex(hash));
  }

  bool acceptBlockHeader(const block_t& header, ValidationState& state) {
    return acceptBlockHeader(std::make_shared<block_t>(header), state);
  }

  bool acceptBlockHeader(std::shared_ptr<block_t> header,
                         ValidationState& state) {
    VBK_ASSERT(header);
    auto* prev = getBlockIndex(header->getPreviousBlock());
    if (prev == nullptr) {
      return state.Invalid(
          block_t::name() + "-bad-prev-block",
          "can not find previous block: " + HexStr(header->getPreviousBlock()));
    }

    auto index = insertBlockHeader(std::move(header), prev);
    VBK_ASSERT(index != nullptr &&
               "insertBlockHeader should have never returned nullptr");

    return true;
  }

  const block_tree_t& getStableTree() const { return *tree_; }

  /**
   * Remove blocks from the temp store that are already in the stable tree
   */
  void cleanUpStaleBlocks() {
    for (auto it = temp_blocks_.begin(); it != temp_blocks_.end();) {
      auto short_hash = tree_->makePrevHash(it->second->getHash());
      auto* index = tree_->getBlockIndex(short_hash);
      it = index ? temp_blocks_.erase(it) : std::next(it);
    }
  }

  void clear() { temp_blocks_.clear(); }

 private:
  index_t* insertBlockHeader(std::shared_ptr<block_t> header, index_t* prev) {
    auto hash = header->getHash();
    index_t* current = getBlockIndex(hash);
    if (current != nullptr) {
      // it is a duplicate
      return current;
    }

    current = doInsertBlockHeader(std::move(header), prev);
    VBK_ASSERT(current != nullptr);

    return current;
  }

  index_t* doInsertBlockHeader(std::shared_ptr<block_t> header, index_t* prev) {
    VBK_ASSERT(header != nullptr);
    index_t* current = touchBlockIndex(header->getHash());
    current->setHeader(std::move(header));
    current->pprev = prev;

    if (current->pprev != nullptr) {
      // prev block found
      current->setHeight(current->pprev->getHeight() + 1);
    } else {
      current->setHeight(0);
    }

    return current;
  }

  index_t* touchBlockIndex(const typename block_t::hash_t& hash) {
    auto shortHash = tree_->makePrevHash(hash);
    auto it = temp_blocks_.find(shortHash);
    if (it != temp_blocks_.end()) {
      return it->second.get();
    }

    // intentionally do not pass 'prev' here - we don't want 'stable' blocks
    // have pnext ptr to temp blocks.
    auto newIndex = make_unique<wrapped_index_t>(0);
    newIndex->setNull();
    it = temp_blocks_.emplace(shortHash, std::move(newIndex)).first;
    return it->second.get();
  }

 private:
  block_index_t temp_blocks_;
  const block_tree_t* tree_;
};

}  // namespace altintegration

#endif
