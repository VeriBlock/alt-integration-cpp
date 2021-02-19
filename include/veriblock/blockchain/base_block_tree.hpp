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
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/tree_algo.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/signals.hpp>

#include "veriblock/fmt.hpp"

namespace altintegration {

/**
 * Base block tree that stores all blocks, maintains tree tips, maintains
 * active chain.
 * @tparam Block
 */
template <typename Block>
struct BaseBlockTree {
  using block_t = Block;
  using block_height_t = typename block_t::height_t;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = typename Block::prev_hash_t;
  using index_t = BlockIndex<Block>;
  using on_invalidate_t = void(const index_t&);
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::unique_ptr<index_t>>;

  const std::unordered_set<index_t*>& getTips() const { return tips_; }
  const block_index_t& getBlocks() const { return blocks_; }

  virtual ~BaseBlockTree() = default;

  BaseBlockTree() = default;
  BaseBlockTree(const BaseBlockTree&) = delete;
  BaseBlockTree& operator=(const BaseBlockTree&) = delete;

  /**
   * Getter for currently Active Chain.
   * @return reference for current chain.
   */
  const Chain<index_t>& getBestChain() const { return this->activeChain_; }

  // HACK: see big comment in VBK tree.
  template <typename T>
  inline prev_block_hash_t makePrevHash(const T& h) const {
    // given any type T, just return an implicit cast to prev_block_hash_t
    return h;
  }

  /**
   * Get BlockIndex by block hash.
   * @tparam T block type
   * @param[in] hash block hash
   * @return nullptr if block is not found, or ptr to block otherwise.
   */
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
      return state.Invalid(block_t::name() + "-no-tip");
    }

    this->overrideTip(*tip);
    return true;
  }

  /**
   * Efficiently connects BlockIndex to this tree, when it is loaded from disk.
   * @param[in] index block to be connected
   * @param[out] state validation state
   * @return true if block is valid and successfully loaded, false otherwise.
   * @invariant NOT atomic. If returned false, leaves BaseBlockTree in undefined
   * state.
   */
  virtual bool loadBlock(const index_t& index, ValidationState& state) {
    VBK_ASSERT(isBootstrapped() && "should be bootstrapped");

    auto currentHash = index.getHash();
    auto* current = getBlockIndex(currentHash);
    // we can not load a block, which already exists on chain and is not a
    // bootstrap block
    if (current && !current->hasFlags(BLOCK_BOOTSTRAP)) {
      return state.Invalid("block-exists");
    }

    // if current block is not known, and previous also not known
    if (!current && !getBlockIndex(index.getHeader().getPreviousBlock())) {
      return state.Invalid("bad-prev");
    }

    current = touchBlockIndex(currentHash);
    VBK_ASSERT(current);

    // touchBlockIndex may return existing block (one of bootstrap blocks), so
    // backup its 'pnext'
    auto next = current->pnext;

    // copy all fields
    *current = index;
    // clear inmem fields
    current->setNullInmemFields();
    current->unsetDirty();
    // recover pnext
    current->pnext = next;
    // recover pprev
    current->pprev = getBlockIndex(index.getHeader().getPreviousBlock());

    if (current->pprev != nullptr) {
      // prev block found
      auto expectedHeight = current->pprev->getHeight() + 1;
      if (current->getHeight() != expectedHeight) {
        return state.Invalid("bad-height");
      }

      current->pprev->pnext.insert(current);
    }

    current->raiseValidity(BLOCK_VALID_TREE);
    current->unsetDirty();

    tryAddTip(current);

    return true;
  }

  /**
   * Removes block and all its successors.
   * @param toRemove block to be removed.
   * @warning fails on assert if unknown hash is provided
   */
  virtual void removeSubtree(index_t& toRemove) {
    VBK_LOG_DEBUG("remove subtree %s", toRemove.toPrettyString());
    // save ptr to a previous block
    auto* prev = toRemove.pprev;
    VBK_ASSERT(prev && "cannot remove the genesis block");

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

  //! @overload
  //! @invariant block must exist in a tree
  void removeSubtree(const hash_t& toRemove) {
    auto* index = getBlockIndex(toRemove);
    VBK_ASSERT_MSG(
        index, "cannot find the subtree to remove: %s", HexStr(toRemove));
    return this->removeSubtree(*index);
  }

  //! @overload
  void removeLeaf(index_t& toRemove) {
    VBK_ASSERT_MSG(toRemove.pnext.empty(),
                   "not a leaf block %s, pnext.size=%d",
                   toRemove.toPrettyString(),
                   toRemove.pnext.size());
    return this->removeSubtree(toRemove);
  }

  /**
   * Mark given block as invalid. Also marks all successors as FAILED_CHILD.
   * @param[in] toBeInvalidated block to be invalidated
   * @param[in] reason invalidation reason. BLOCK_FAILED_BLOCK is used to
   * indicate that block is invalid because of consensus rules (altchain decided
   * to invalidate it). BLOCK_FAILED_POP is used to indicate that block is
   * invalid because it contains invalid POP data.
   * @param[in] shouldDetermineBestChain when true, will trigger fork resolution
   * with all tips. If it is known that block is on a fork (not on active
   * chain), it is safe to omit fork resolution here. Otherwise, leaves tree in
   * undefined state.
   */
  void invalidateSubtree(index_t& toBeInvalidated,
                         enum BlockValidityStatus reason,
                         bool shouldDetermineBestChain = true) {
    VBK_LOG_INFO("Invalidating %s subtree: reason=%d block=%s",
                 block_t::name(),
                 (int)reason,
                 toBeInvalidated.toShortPrettyString());

    VBK_ASSERT(toBeInvalidated.pprev && "cannot invalidate the genesis block");

    VBK_ASSERT(isValidInvalidationReason(reason) &&
               "invalid invalidation reason");

    if (toBeInvalidated.hasFlags(reason)) {
      return;
    }

    // all descendants of an invalid block are already flagged as
    // BLOCK_FAILED_CHILD
    if (!toBeInvalidated.isValid()) {
      doInvalidate(toBeInvalidated, reason);
      return;
    }

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

    // after invalidation, the previous block might have become a tip
    tryAddTip(toBeInvalidated.pprev);

    if (shouldDetermineBestChain) {
      updateTips();
    }
  }

  //! @overload
  //! @invariant block must exist in a tree
  void invalidateSubtree(const hash_t& toBeInvalidated,
                         enum BlockValidityStatus reason,
                         bool shouldDetermineBestChain = true) {
    auto* index = getBlockIndex(toBeInvalidated);
    VBK_ASSERT(index && "cannot find the subtree to invalidate");
    return invalidateSubtree(*index, reason, shouldDetermineBestChain);
  }

  void revalidateSubtree(const hash_t& hash,
                         enum BlockValidityStatus reason,
                         bool shouldDetermineBestChain = true) {
    auto* index = this->getBlockIndex(hash);
    VBK_ASSERT(index && "cannot find the subtree to revalidate");
    revalidateSubtree(*index, reason, shouldDetermineBestChain);
  }

  /*
   * `tobeValidated` does NOT necessarily become valid after a call to this
   * function.
   */
  void revalidateSubtree(index_t& toBeValidated,
                         enum BlockValidityStatus reason,
                         bool shouldDetermineBestChain = true) {
    VBK_LOG_DEBUG("Revalidating %s subtree: reason=%d block=%s",
                  block_t::name(),
                  (int)reason,
                  toBeValidated.toShortPrettyString());

    VBK_ASSERT(toBeValidated.pprev && "cannot revalidate the genesis block");
    VBK_ASSERT(isValidInvalidationReason(reason) &&
               "invalid revalidation reason");

    if (!toBeValidated.hasFlags(reason)) {
      return;
    }

    // if the block has any invalidity flags other than `reason`, its
    // descendants are already flagged as BLOCK_FAILED_CHILD and should stay so
    if (toBeValidated.hasFlags(static_cast<enum BlockValidityStatus>(
            BLOCK_FAILED_MASK & ~reason))) {
      doReValidate(toBeValidated, reason);
      return;
    }

    doReValidate(toBeValidated, reason);

    for (auto* pnext : toBeValidated.pnext) {
      forEachNodePreorder<block_t>(*pnext, [&](index_t& index) -> bool {
        doReValidate(index, BLOCK_FAILED_CHILD);
        bool valid = index.isValid();
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
  bool isBootstrapped() const {
    if (!blocks_.empty() && activeChain_.tip() != nullptr) {
      return true;
    }

    if (blocks_.empty() && activeChain_.tip() == nullptr) {
      return false;
    }

    VBK_ASSERT_MSG(
        false,
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

  index_t& getRoot() const {
    VBK_ASSERT_MSG(isBootstrapped(), "must be bootstrapped");
    return *getBestChain().first();
  }

  //! the number of blocks that have BLOCK_APPLIED flag set
  block_height_t appliedBlockCount = 0;

 protected:
  virtual void determineBestChain(index_t& candidate,
                                  ValidationState& state) = 0;

  void tryAddTip(index_t* index) {
    VBK_ASSERT(index);

    if (!index->isValidTip()) {
      return;
    }

    auto it = tips_.find(index->pprev);
    if (it != tips_.end()) {
      // remove the previous block from the valid tip set as it can no longer be
      // a valid tip
      tips_.erase(it);
    }

    tips_.insert(index);
  }

  index_t* touchBlockIndex(const hash_t& hash) {
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    if (it != blocks_.end()) {
      return it->second.get();
    }

    std::unique_ptr<index_t> newIndex = nullptr;
    auto itr = removed_.find(shortHash);
    if (itr != removed_.end()) {
      newIndex = std::move(itr->second);
      removed_.erase(itr);
    } else {
      newIndex = std::unique_ptr<index_t>(new index_t{});
    }

    newIndex->setNull();
    it = blocks_.insert({shortHash, std::move(newIndex)}).first;
    return it->second.get();
  }

  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header,
                               block_height_t bootstrapHeight = 0) {
    VBK_ASSERT(header != nullptr);

    index_t* current = touchBlockIndex(header->getHash());
    current->setHeader(std::move(header));
    current->pprev = getBlockIndex(header->getPreviousBlock());

    if (current->pprev != nullptr) {
      // prev block found
      current->setHeight(current->pprev->getHeight() + 1);
      auto pair = current->pprev->pnext.insert(current);
      VBK_ASSERT(pair.second && "block already existed in prev");

      if (!current->pprev->isValid()) {
        current->setFlag(BLOCK_FAILED_CHILD);
      }
    } else {
      VBK_ASSERT(activeChain_.tip() == nullptr && blocks_.size() == 1);
      current->setHeight(bootstrapHeight);
    }

    tryAddTip(current);

    return current;
  }

  index_t* insertBlockHeader(const std::shared_ptr<block_t>& block,
                             block_height_t bootstrapHeight = 0) {
    assertBlockSanity(*block);
    auto hash = block->getHash();
    index_t* current = getBlockIndex(hash);
    if (current != nullptr) {
      // it is a duplicate
      return current;
    }

    current = doInsertBlockHeader(block, bootstrapHeight);
    this->onBlockInserted(current);

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
  //! @private
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
  //! callback which is executed when new block is added to a tree
  virtual void onBlockInserted(index_t* /*ignore*/) {
    /* do nothing in base tree */
  }

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
    auto tips = findValidTips<block_t>(this->getTips(), modifiedBlock);
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
    for (auto* tip : tips_) {
      VBK_ASSERT_MSG(tip->isValidTip(),
                     "found block %s in tips_ which is not a valid tip",
                     tip->toPrettyString());

      ValidationState state;
      determineBestChain(*tip, state);
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

  void removeSingleBlock(index_t& block) {
    // if it is a tip, we also remove it
    tips_.erase(&block);

    if (block.pprev != nullptr) {
      block.pprev->pnext.erase(&block);
    }

    auto shortHash = makePrevHash(block.getHash());
    auto& it = blocks_.at(shortHash);

    it->setNull();
    removed_[shortHash] = std::move(it);
    blocks_.erase(shortHash);
  }

  void doInvalidate(index_t& block, enum BlockValidityStatus reason) {
    VBK_ASSERT_MSG(
        !block.isValidUpTo(BLOCK_CAN_BE_APPLIED) ||
            (reason & BLOCK_FAILED_POP) == 0u,
        "attempted to set mutually exclusive flags BLOCK_CAN_BE_APPLIED "
        "and BLOCK_FAILED_POP for block %s",
        block.toPrettyString());

    block.setFlag(reason);
    tips_.erase(&block);

    validity_sig_.emit(block);
  }

  void doReValidate(index_t& block, enum BlockValidityStatus reason) {
    block.unsetFlag(reason);
    tryAddTip(&block);
    validity_sig_.emit(block);
  }

 protected:
  //! stores ALL blocks, including valid and invalid
  block_index_t blocks_;
  //! stores all removed blocks, to ensure pointers to blocks remain stable
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
