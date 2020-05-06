// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_BASE_BLOCK_TREE_HPP
#define ALTINTEGRATION_BASE_BLOCK_TREE_HPP

#include <unordered_map>
#include <unordered_set>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/tree_algo.hpp>

namespace altintegration {

/**
 * A base block tree that stores all blocks, maintains tree tips, maintains
 * active chain.
 * @tparam Block
 */
template <typename Block>
struct BaseBlockTree {
  using block_t = Block;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = decltype(Block::previousBlock);
  using index_t = BlockIndex<Block>;
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::shared_ptr<index_t>>;

  const std::unordered_set<index_t*>& getTips() const { return tips_; }
  const block_index_t& getBlocks() const { return blocks_; }

  virtual ~BaseBlockTree() = default;

  const Chain<index_t>& getBestChain() const { return this->activeChain_; }

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, hash_t>::value ||
                std::is_same<T, prev_block_hash_t>::value>::type>
  index_t* getBlockIndex(const T& hash) const {
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    return it == blocks_.end() ? nullptr : it->second.get();
  }

  void removeSubtree(const hash_t& toRemove,
                     bool shouldDetermineBestChain = true) {
    auto* index = getBlockIndex(toRemove);
    if (!index) {
      return;
    }
    return removeSubtree(*index, shouldDetermineBestChain);
  }

  void removeSubtree(index_t& toRemove, bool shouldDetermineBestChain = true) {
    // save ptr to a previous block
    auto* prev = toRemove.pprev;
    assert(prev);

    bool isOnMainChain = activeChain_.contains(&toRemove);
    if (isOnMainChain) {
      ValidationState dummy;
      this->setTip(*toRemove.pprev, dummy, false);
    }

    // remove this block from 'pnext' set of previous block
    if (prev) {
      prev->pnext.erase(&toRemove);
    }

    forEachNodePostorder<block_t>(
        toRemove, [&](index_t& next) { removeSingleBlock(next); });

    // after removal, try to add tip
    tryAddTip(prev);

    if (isOnMainChain) {
      updateTips(shouldDetermineBestChain);
    }
  }

  void invalidateSubtree(const hash_t& toBeInvalidated,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    auto* index = getBlockIndex(toBeInvalidated);
    if (!index) {
      return;
    }
    return invalidateSubtree(*index, reason, shouldDetermineBestChain);
  }

  void invalidateSubtree(index_t& toBeInvalidated,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    assert(toBeInvalidated.pprev);
    bool isOnMainChain = activeChain_.contains(&toBeInvalidated);
    if (isOnMainChain) {
      ValidationState dummy;
      bool ret = this->setTip(*toBeInvalidated.pprev, dummy, false);
      assert(ret);
      (void)ret;
    }

    toBeInvalidated.setFlag(reason);

    // flag next subtrees (excluding current block)  as BLOCK_FAILED_CHILD
    forEachNextNodePreorder<block_t>(toBeInvalidated,
                                     [&](index_t& index) -> bool {
                                       bool shouldContinue = index.isValid();
                                       index.setFlag(BLOCK_FAILED_CHILD);
                                       return shouldContinue;
                                     });

    // after invalidation, try to add tip
    tryAddTip(toBeInvalidated.pprev);

    if (isOnMainChain) {
      updateTips(shouldDetermineBestChain);
    }
  }

 protected:
  virtual void determineBestChain(Chain<index_t>& currentBest,
                                  index_t& indexNew,
                                  ValidationState& state,
                                  bool isBootstrap) = 0;

  void tryAddTip(index_t* index) {
    assert(index);

    auto it = tips_.find(index->pprev);
    if (it != tips_.end()) {
      // we found prev block in chainTips
      tips_.erase(it);
    }

    if (index->isValidTip()) {
      tips_.insert(index);
    }
  }

  index_t* touchBlockIndex(const hash_t& hash) {
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    if (it != blocks_.end()) {
      return it->second.get();
    }

    std::shared_ptr<index_t> newIndex;
    auto itr = removed_.find(shortHash);
    if (itr != removed_.end()) {
      newIndex = itr->second;
      removed_.erase(itr);
    } else {
      newIndex = std::make_shared<index_t>();
    }

    it = blocks_.insert({shortHash, std::move(newIndex)}).first;
    return it->second.get();
  }

  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header) {
    assert(header != nullptr);

    index_t* current = touchBlockIndex(header->getHash());
    current->header = header;
    current->pprev = getBlockIndex(header->previousBlock);

    if (current->pprev != nullptr) {
      // prev block found
      current->height = current->pprev->height + 1;
      current->pprev->pnext.insert(current);

      if (!current->pprev->isValid()) {
        current->setFlag(BLOCK_FAILED_CHILD);
      }
    } else {
      current->height = 0;
    }

    tryAddTip(current);

    return current;
  }

  //! updates tree tip
  virtual bool setTip(index_t& to, ValidationState&, bool) {
    activeChain_.setTip(&to);
    return true;
  }

  std::string toPrettyString(size_t level = 0) const {
    auto tip = activeChain_.tip();
    std::ostringstream ss;
    std::string pad(level, ' ');
    ss << pad << "{tip=" << (tip ? tip->toPrettyString() : "<empty>") << "}\n";
    ss << pad << "{blocks=\n";
    // sort blocks by height
    std::vector<std::pair<int, index_t*>> byheight;
    byheight.reserve(blocks_.size());
    for (const auto& p : blocks_) {
      byheight.push_back({p.second->height, p.second.get()});
    }
    std::sort(byheight.rbegin(), byheight.rend());
    for (const auto& p : byheight) {
      ss << p.second->toPrettyString(level + 2) << "\n";
    }
    ss << pad << "}\n";
    ss << pad << "{tips=\n";
    for (const auto* _tip : tips_) {
      ss << _tip->toPrettyString(level + 2) << "\n";
    }
    ss << pad << "}";
    return ss.str();
  }

 private:
  void updateTips(bool shouldDetermineBestChain = true) {
    for (auto it = tips_.begin(); it != tips_.end();) {
      index_t* index = *it;
      if (!index->isValid()) {
        it = tips_.erase(it);
      } else {
        if (shouldDetermineBestChain) {
          ValidationState state;
          determineBestChain(
              activeChain_, *index, state, /*isBootstrap=*/false);
        }
        ++it;
      }
    }
  }

  // HACK: see comment below.
  template <typename T>
  inline prev_block_hash_t makePrevHash(const T& h) const {
    // given any type T, just return an implicit cast to prev_block_hash_t
    return h;
  }

  void removeSingleBlock(index_t& block) {
    // if it is a tip, we also remove it
    tips_.erase(&block);
    block.pnext.clear();

    auto shortHash = makePrevHash(block.getHash());
    auto it = blocks_.at(shortHash);
    removed_[shortHash] = it;
    blocks_.erase(shortHash);
  }

 protected:
  //! stores ALL blocks, including valid and invalid
  block_index_t blocks_;
  //! stores all removed blocks, to ensure pointers to blocks remain stable
  // TODO(bogdan): remove for future releases
  block_index_t removed_;
  //! stores ONLY VALID tips
  std::unordered_set<index_t*> tips_;
  //! currently applied chain
  Chain<index_t> activeChain_;
};

// HACK: getBlockIndex accepts either hash_t or prev_block_hash_t
// then, depending on what it received, it should do trim LE on full hash to
// receive short hash, which is stored inside a map. In this weird case, when
// Block=VbkBlock, we may call `getBlockIndex(block->previousBlock)`, it is a
// call `getBlockIndex(Blob<12>). But when `getBlockIndex` accepts it, it does
// an implicit cast to full hash (hash_t), adding zeroes in the end. Then,
// .trimLE returns 12 zeroes.
//
// This hack allows us to inject explicit conversion hash_t (Blob<24>) ->
// prev_block_hash_t (Blob<12>).
template <>
template <>
inline BaseBlockTree<VbkBlock>::prev_block_hash_t
BaseBlockTree<VbkBlock>::makePrevHash<BaseBlockTree<VbkBlock>::hash_t>(
    const hash_t& h) const {
  // do an explicit cast from hash_t -> prev_block_hash_t
  return h.template trimLE<prev_block_hash_t::size()>();
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_BASE_BLOCK_TREE_HPP
