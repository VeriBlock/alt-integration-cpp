// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TEMP_BLOCK_TREE_HPP
#define ALTINTEGRATION_TEMP_BLOCK_TREE_HPP

#include <memory>

#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

template <typename StableBlockTree>
struct TempBlockTree {
  using block_tree_t = StableBlockTree;
  using block_t = typename block_tree_t::block_t;
  using index_t = typename block_tree_t::index_t;
  using block_index_t = typename block_tree_t::base::block_index_t;

  TempBlockTree(const block_tree_t& tree) : tree_(&tree) {}

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

  bool acceptBlock(const block_t& header, ValidationState& state) {
    return acceptBlock(std::make_shared<block_t>(header), state);
  }

  bool acceptBlock(std::shared_ptr<block_t> header, ValidationState& state) {
    VBK_ASSERT(header);
    auto* prev = getBlockIndex(header->getPreviousBlock());

    if (prev == nullptr) {
      return state.Invalid(
          block_t::name() + "-bad-prev-block",
          "can not find previous block: " + HexStr(header->getPreviousBlock()));
    }

    auto index = insertBlockHeader(header);
    VBK_ASSERT(index != nullptr &&
               "insertBlockHeader should have never returned nullptr");

    return true;
  }

  const block_tree_t& getStableTree() const { return *tree_; }

  /**
   * Remove blocks from the temp store that are already in the stable tree
   */
  void cleanUp() {
    for (auto it = temp_blocks_.begin(); it != temp_blocks_.end();) {
      auto short_hash = tree_->makePrevHash(it->second->getHash());
      auto* index = tree_->getBlockIndex(short_hash);
      it = index ? temp_blocks_.erase(it) : std::next(it);
    }
  }

  void clear() { temp_blocks_.clear(); }

 private:
  index_t* insertBlockHeader(const std::shared_ptr<block_t>& header) {
    auto hash = header->getHash();
    index_t* current = getBlockIndex(hash);
    if (current != nullptr) {
      // it is a duplicate
      return current;
    }

    current = doInsertBlockHeader(header);
    VBK_ASSERT(current != nullptr);

    return current;
  }

  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header) {
    VBK_ASSERT(header != nullptr);
    index_t* current = touchBlockIndex(header->getHash());
    current->setHeader(std::move(header));
    current->pprev = getBlockIndex(header->getPreviousBlock());

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

    auto newIndex = std::unique_ptr<index_t>(new index_t{});

    newIndex->setNull();
    it = temp_blocks_.insert({shortHash, std::move(newIndex)}).first;
    return it->second.get();
  }

 private:
  block_index_t temp_blocks_;
  const block_tree_t* tree_;
};

}  // namespace altintegration

#endif