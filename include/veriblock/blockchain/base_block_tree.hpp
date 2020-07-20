// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_BASE_BLOCK_TREE_HPP
#define ALTINTEGRATION_BASE_BLOCK_TREE_HPP

#include <unordered_map>
#include <unordered_set>
#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/tree_algo.hpp>
#include <veriblock/comparator.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/signals.hpp>

#include "veriblock/fmt.hpp"

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
  using prev_block_hash_t = typename Block::prev_hash_t;
  using index_t = BlockIndex<Block>;
  using on_invalidate_t = void(const index_t&);
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::shared_ptr<index_t>>;

  const std::unordered_set<index_t*>& getTips() const { return tips_; }
  const block_index_t& getBlocks() const { return blocks_; }

  virtual ~BaseBlockTree() = default;

  BaseBlockTree() = default;
  BaseBlockTree(const BaseBlockTree&) = delete;
  BaseBlockTree& operator=(const BaseBlockTree&) = delete;

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

  virtual bool loadTip(const hash_t& hash, ValidationState& state) {
    auto* tip = getBlockIndex(hash);
    if (!tip) {
      return state.Error(block_t::name() + "-no-tip");
    }

    this->overrideTip(*tip);
    return true;
  }

  //! @returns false if block is failed to pass validation. State will be set to
  //! Error if block failed to meet expectations (block is invalid, and some
  //! field has unexpected value, therefore we need to perform reindex).
  //! @invariant NOT atomic.
  virtual bool loadBlock(const index_t& index, ValidationState& state) {
    auto currentHash = index.getHash();
    auto* current = getBlockIndex(currentHash);
    // we can not load a block, which already exists on chain and is not a
    // bootstrap block
    if (current && !current->hasFlags(BLOCK_BOOTSTRAP)) {
      return state.Error("block-exists");
    }

    // if current block is not known, and previous also not known
    if (!current && !getBlockIndex(index.getHeader().previousBlock)) {
      return state.Error("bad-prev");
    }

    current = touchBlockIndex(currentHash);
    VBK_ASSERT(current);

    // touchBlockIndex may return existing block (one of bootstrap blocks), so
    // backup its 'pnext'
    auto next = current->pnext;
    auto status = current->status;

    // copy all fields
    *current = index;
    // recover status
    current->status = status;
    // recover pnext
    current->pnext = next;
    // recover pprev
    current->pprev = getBlockIndex(index.getHeader().previousBlock);

    if (current->pprev != nullptr) {
      // prev block found
      auto expectedHeight = current->pprev->getHeight() + 1;
      if (current->getHeight() != expectedHeight) {
        return state.Error("bad-height");
      }

      current->pprev->pnext.insert(current);
    }

    current->setFlag(BLOCK_VALID_TREE);
    current->unsetDirty();

    tryAddTip(current);

    return true;
  }

  void removeSubtree(const hash_t& toRemove) {
    auto* index = getBlockIndex(toRemove);
    VBK_ASSERT(index && "can not find the subtree to remove");
    return this->removeSubtree(*index);
  }

  virtual void removeSubtree(index_t& toRemove) {
    VBK_LOG_DEBUG("remove subtree %s", toRemove.toPrettyString());
    // save ptr to a previous block
    auto* prev = toRemove.pprev;
    VBK_ASSERT(prev && "can not remove genesis block");

    bool isOnMainChain = activeChain_.contains(&toRemove);
    if (isOnMainChain) {
      ValidationState dummy;
      bool success = this->setState(*prev, dummy);
      VBK_ASSERT(success);
    }

    // remove this block from 'pnext' set of previous block
    prev->pnext.erase(&toRemove);
    forEachNodePostorder<block_t>(
        toRemove, [&](index_t& next) { removeSingleBlock(next); });

    // after removal, try to add tip
    tryAddTip(prev);

    if (isOnMainChain) {
      updateTips();
    }
  }

  void removeLeaf(index_t& toRemove) {
    VBK_ASSERT(toRemove.pnext.empty() && "not a leaf block");
    return this->removeSubtree(toRemove);
  }

  void invalidateSubtree(const hash_t& toBeInvalidated,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    auto* index = getBlockIndex(toBeInvalidated);
    VBK_ASSERT(index && "could not find the subtree to invalidate");
    return invalidateSubtree(*index, reason, shouldDetermineBestChain);
  }

  void invalidateSubtree(index_t& toBeInvalidated,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    VBK_LOG_INFO("Invalidating %s subtree: reason=%d block=%s",
                 block_t::name(),
                 (int)reason,
                 toBeInvalidated.toShortPrettyString());
    VBK_ASSERT(toBeInvalidated.pprev && "can not invalidate genesis block");
    bool isOnMainChain = activeChain_.contains(&toBeInvalidated);
    if (isOnMainChain) {
      ValidationState dummy;
      bool success = this->setState(*toBeInvalidated.pprev, dummy);
      VBK_ASSERT(success);
    }

    doInvalidate(toBeInvalidated, reason);

    // flag next subtrees (excluding current block) as BLOCK_FAILED_CHILD
    for (auto* ptr : toBeInvalidated.pnext) {
      auto* pnext = ptr;
      forEachNodePreorder<block_t>(*pnext, [&](index_t& index) {
        bool valid = index.isValid();
        doInvalidate(index, BLOCK_FAILED_CHILD);
        return valid;
      });
    }

    // after invalidation, try to add tip
    tryAddTip(toBeInvalidated.pprev);

    if (shouldDetermineBestChain) {
      updateTips();
    }
  }

  void revalidateSubtree(const hash_t& hash,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    auto* index = this->getBlockIndex(hash);
    VBK_ASSERT(index && "can not find subtree to revalidate");
    revalidateSubtree(*index, reason, shouldDetermineBestChain);
  }

  void revalidateSubtree(index_t& toBeValidated,
                         enum BlockStatus reason,
                         bool shouldDetermineBestChain = true) {
    VBK_LOG_INFO("Revalidating %s subtree: reason=%d block=%s",
                 block_t::name(),
                 (int)reason,
                 toBeValidated.toShortPrettyString());
    doReValidate(toBeValidated, reason);
    tryAddTip(&toBeValidated);

    for (auto* pnext : toBeValidated.pnext) {
      forEachNodePreorder<block_t>(*pnext, [&](index_t& index) -> bool {
        doReValidate(index, BLOCK_FAILED_CHILD);
        bool valid = index.isValid();
        tryAddTip(&index);
        return valid;
      });
    }

    if (shouldDetermineBestChain) {
      updateTips();
    }
  }

  /**
   * Check if the blockchain is bootstrapped
   *
   * @return true if the blockchain is bootstrapped, false otherwise
   */
  bool isBootstrapped() {
    if (!blocks_.empty() && activeChain_.tip() != nullptr) {
      return true;
    }

    if (blocks_.empty() && activeChain_.tip() == nullptr) {
      return false;
    }

    VBK_ASSERT(
        false &&
        "state corruption: the blockchain is neither bootstrapped nor empty");
    return false;
  }

  virtual bool setState(const hash_t& block, ValidationState& state) {
    auto* index = getBlockIndex(block);
    if (!index) {
      return state.Invalid(block_t::name() + "-setstate-unknown-block",
                           "could not find the block to set the state to");
    }
    return setState(*index, state);
  }

  virtual bool setState(index_t& index, ValidationState&) {
    overrideTip(index);
    return true;
  }

  virtual void overrideTip(index_t& to) {
    VBK_LOG_DEBUG("SetTip=%s", to.toPrettyString());
    activeChain_.setTip(&to);
    tryAddTip(&to);
  }

  //! connects a handler to a signal 'On Invalidate Block'
  size_t connectOnValidityBlockChanged(
      const std::function<on_invalidate_t>& f) {
    return validity_sig_.connect(f);
  }

  //! disconnects a handler to a signal 'On Invalidate Block'
  bool disconnectOnValidityBlockChanged(size_t id) {
    return validity_sig_.disconnect(id);
  }

  bool operator==(const BaseBlockTree& o) const {
    CollectionOfPtrComparator cmp{};
    bool a = cmp(blocks_, o.blocks_);
    bool b = cmp(tips_, o.tips_);
    bool c = (activeChain_ == o.activeChain_);
    return a && b && c;
  }

  bool operator!=(const BaseBlockTree& o) const { return !operator==(o); }

 protected:
  virtual void determineBestChain(index_t& candidate,
                                  ValidationState& state) = 0;

  void tryAddTip(index_t* index) {
    VBK_ASSERT(index);

    if (!index->isValid()) {
      return;
    }

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

    std::shared_ptr<index_t> newIndex = nullptr;
    auto itr = removed_.find(shortHash);
    if (itr != removed_.end()) {
      newIndex = itr->second;
      removed_.erase(itr);
    } else {
      newIndex = std::make_shared<index_t>();
    }

    newIndex->setNull();
    it = blocks_.insert({shortHash, std::move(newIndex)}).first;
    return it->second.get();
  }

  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header) {
    VBK_ASSERT(header != nullptr);

    index_t* current = touchBlockIndex(header->getHash());
    current->setHeader(std::move(header));
    current->pprev = getBlockIndex(header->previousBlock);

    if (current->pprev != nullptr) {
      // prev block found
      current->setHeight(current->pprev->getHeight() + 1);
      auto pair = current->pprev->pnext.insert(current);
      VBK_ASSERT(pair.second && "block already existed in prev");

      if (!current->pprev->isValid()) {
        current->setFlag(BLOCK_FAILED_CHILD);
      }
    } else {
      current->setHeight(0);
    }

    tryAddTip(current);

    return current;
  }

  index_t* insertBlockHeader(const std::shared_ptr<block_t>& block) {
    auto hash = block->getHash();
    index_t* current = getBlockIndex(hash);
    if (current != nullptr) {
      // it is a duplicate
      return current;
    }

    current = doInsertBlockHeader(block);
    if (current->pprev) {
      current->chainWork = current->pprev->chainWork + getBlockProof(*block);
    } else {
      current->chainWork = getBlockProof(*block);
    }

    // raise validity may return false if block is invalid
    current->raiseValidity(BLOCK_VALID_TREE);
    return current;
  }

  std::string toPrettyString(size_t level = 0) const {
    auto tip = activeChain_.tip();
    std::string pad(level, ' ');

    std::string blocksStr{};
    // sort blocks by height
    std::vector<std::pair<int, index_t*>> byheight;
    byheight.reserve(blocks_.size());
    for (const auto& p : blocks_) {
      byheight.push_back({p.second->getHeight(), p.second.get()});
    }
    std::sort(byheight.rbegin(), byheight.rend());
    for (const auto& p : byheight) {
      blocksStr += (p.second->toPrettyString(level + 2) + "\n");
    }

    std::string tipsStr{};
    for (const auto* _tip : tips_) {
      tipsStr += (_tip->toPrettyString(level + 2) + "\n");
    }

    return fmt::sprintf("%s{tip=%s}\n%s{blocks=\n%s%s}\n%s{tips=\n%s%s}",
                        pad,
                        (tip ? tip->toPrettyString() : "<empty>"),
                        pad,
                        blocksStr,
                        pad,
                        pad,
                        tipsStr,
                        pad);
  }

 public:
  class DeferForkResolutionGuard {
    BaseBlockTree<Block>& tree_;

   public:
    DeferForkResolutionGuard(BaseBlockTree<Block>& tree) : tree_(tree) {
      tree_.deferForkResolution();
    }

    void overrideDeferredForkResolution(index_t* bestChain) {
      // there's no obvious way to go back to having no best chain, so we
      // just have to let fork resolution run its course
      if (bestChain != nullptr) {
        return tree_.overrideDeferredForkResolution(*bestChain);
      }
    }

    ~DeferForkResolutionGuard() { tree_.continueForkResolution(); }
  };

  DeferForkResolutionGuard deferForkResolutionGuard() {
    return DeferForkResolutionGuard(*this);
  }

 protected:
  /**
   * Find all tips affected by a block modification and schedule or do fork
   * resolution
   */
  void updateAffectedTips(index_t& modifiedBlock) {
    if (deferForkResolutionDepth == 0) {
      ValidationState dummy;
      return doUpdateAffectedTips(modifiedBlock, dummy);
    }

    if (!isUpdateTipsDeferred) {
      if (lastModifiedBlock == nullptr) {
        lastModifiedBlock = &modifiedBlock;
      } else {
        isUpdateTipsDeferred = true;
        lastModifiedBlock = nullptr;
      }
    }
  }

  /**
   * Find all tips affected by a block modification and do fork resolution
   */
  void doUpdateAffectedTips(index_t& modifiedBlock, ValidationState& state) {
    auto tips = findValidTips<block_t>(modifiedBlock);
    VBK_LOG_DEBUG(
        "Found %d affected valid tips in %s", tips.size(), block_t::name());
    for (auto* tip : tips) {
      determineBestChain(*tip, state);
    }
  }

  void updateTips() {
    if (deferForkResolutionDepth == 0) {
      return doUpdateTips();
    }
    isUpdateTipsDeferred = true;
    lastModifiedBlock = nullptr;
  }

 private:
  int deferForkResolutionDepth = 0;
  bool isUpdateTipsDeferred = false;
  index_t* lastModifiedBlock = nullptr;

  void doUpdateTips() {
    for (auto it = tips_.begin(); it != tips_.end();) {
      index_t* tip = *it;
      if (!tip->isValid()) {
        it = tips_.erase(it);
      } else {
        ValidationState state;
        determineBestChain(*tip, state);
        ++it;
      }
    }
  }

  /**
   * cancel pending fork resolution requests and revert
   * the best chain to a known state
   */
  void overrideDeferredForkResolution(index_t& bestChain) {
    lastModifiedBlock = nullptr;
    isUpdateTipsDeferred = false;

    ValidationState dummy;
    bool success = setState(bestChain, dummy);
    VBK_ASSERT(
        success &&
        "state corruption: could not revert to the saved best chain tip");
  }

  void deferForkResolution() { ++deferForkResolutionDepth; }

  void continueForkResolution() {
    VBK_ASSERT(deferForkResolutionDepth > 0);
    --deferForkResolutionDepth;

    if (deferForkResolutionDepth > 0) {
      return;
    }

    if (lastModifiedBlock != nullptr) {
      VBK_ASSERT(!isUpdateTipsDeferred);
      ValidationState dummy;
      doUpdateAffectedTips(*lastModifiedBlock, dummy);
      lastModifiedBlock = nullptr;
    }

    if (isUpdateTipsDeferred) {
      doUpdateTips();
      isUpdateTipsDeferred = false;
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

    if (block.pprev != nullptr) {
      block.pprev->pnext.erase(&block);
    }

    auto shortHash = makePrevHash(block.getHash());
    auto it = blocks_.at(shortHash);
    // TODO: it is a hack because we do not erase blocks and just move them to
    // the remove_ container
    it->setNull();
    removed_[shortHash] = it;
    blocks_.erase(shortHash);
  }

  void doInvalidate(index_t& block, enum BlockStatus reason) {
    block.setFlag(reason);
    validity_sig_.emit(block);
  }

  void doReValidate(index_t& block, enum BlockStatus reason) {
    block.unsetFlag(reason);
    validity_sig_.emit(block);
  }

 protected:
  //! stores ALL blocks, including valid and invalid
  block_index_t blocks_;
  //! stores all removed blocks, to ensure pointers to blocks remain stable
  // TODO(bogdan): remove for future releases
  block_index_t removed_;
  //! stores ONLY VALID tips, including currently active tip
  std::unordered_set<index_t*> tips_;
  //! currently applied chain
  Chain<index_t> activeChain_;
  //! signals to the end user that block have been invalidated
  signals::Signal<on_invalidate_t> validity_sig_;
};

}  // namespace altintegration
#endif  // ALTINTEGRATION_BASE_BLOCK_TREE_HPP
