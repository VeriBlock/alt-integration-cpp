// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_BASE_BLOCK_TREE_HPP
#define ALTINTEGRATION_BASE_BLOCK_TREE_HPP

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/fmt.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/signals.hpp>
#include <veriblock/pop/storage/block_reader.hpp>
#include <veriblock/pop/storage/stored_block_index.hpp>
#include <veriblock/pop/trace.hpp>

#include "block_index.hpp"
#include "blockchain_util.hpp"
#include "chain.hpp"
#include "payloads_index.hpp"
#include "tree_algo.hpp"

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
  using stored_index_t = StoredBlockIndex<Block>;
  using on_invalidate_t = void(const index_t&);
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::unique_ptr<index_t>>;

  const std::unordered_set<index_t*>& getTips() const { return tips_; }
  std::vector<index_t*> getBlocks() const {
    std::vector<index_t*> blocks;
    blocks.reserve(blocks_.size());
    for (const auto& el : blocks_) {
      if (!el.second->isDeleted()) {
        blocks.push_back(el.second.get());
      }
    }
    return blocks;
  }

  std::vector<index_t*> getAllBlocks() const {
    std::vector<index_t*> blocks;
    blocks.reserve(blocks_.size());
    for (const auto& el : blocks_) {
      blocks.push_back(el.second.get());
    }
    return blocks;
  }

  const BlockReader& getBlockProvider() const { return blockProvider_; }

  virtual ~BaseBlockTree() {
    if (!isBootstrapped()) {
      return;
    }

    deallocateTree(getRoot());
  }

  BaseBlockTree(const BlockReader& blockProvider)
      : blockProvider_(blockProvider) {}

  // non-copyable
  BaseBlockTree(const BaseBlockTree&) = delete;
  BaseBlockTree& operator=(const BaseBlockTree&) = delete;

  // movable
  // NOLINTNEXTLINE(performance-noexcept-move-constructor)
  BaseBlockTree(BaseBlockTree&&) = default;
  // NOLINTNEXTLINE(performance-noexcept-move-constructor)
  BaseBlockTree& operator=(BaseBlockTree&&) = default;

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
  index_t* getBlockIndex(const T& hash) {
    const auto& t = as_const(*this);
    return const_cast<index_t*>(t.getBlockIndex(hash));
  }

  //! @overload
  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, hash_t>::value ||
                std::is_same<T, prev_block_hash_t>::value>::type>
  const index_t* getBlockIndex(const T& hash) const {
    auto blockIndex = findBlockIndex(hash);
    return blockIndex != nullptr && blockIndex->isDeleted() ? nullptr
                                                            : blockIndex;
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
  index_t* findBlockIndex(const T& hash) {
    const auto& t = as_const(*this);
    return const_cast<index_t*>(t.findBlockIndex(hash));
  }

  //! @overload
  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, hash_t>::value ||
                std::is_same<T, prev_block_hash_t>::value>::type>
  const index_t* findBlockIndex(const T& hash) const {
    VBK_TRACE_ZONE_SCOPED;
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    return it == blocks_.end() ? nullptr : it->second.get();
  }

  virtual bool loadTip(const hash_t& hash, ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_ASSERT_MSG(this->isLoadingBlocks_,
                   "%s tree must be in LoadingBlocks state!",
                   block_t::name());
    VBK_ASSERT_MSG(
        !this->isLoaded_, "%s tree is already loaded!", block_t::name());

    auto* tip = getBlockIndex(hash);
    if (tip == nullptr) {
      return state.Invalid(
          block_t::name() + "-no-tip",
          format("tip {} doesn't exist in block tree", HexStr(hash)));
    }

    this->overrideTip(*tip);

    // we no longer can execute loadTip/loadBlocks
    this->isLoaded_ = true;
    this->isLoadingBlocks_ = false;
    return true;
  }

  /**
   * Efficiently connects BlockIndex to this tree as a leaf, when it is loaded
   * from disk.
   * @param[in] index block to be connected
   * @param[in] fast_load flag
   * @param[out] state validation state
   * @return true if block is valid and successfully loaded, false otherwise.
   * @invariant NOT atomic. If returned false, leaves BaseBlockTree in undefined
   * state.
   */
  virtual bool loadBlockForward(const stored_index_t& index,
                                bool fast_load,
                                ValidationState& state) {
    return loadBlockInner(index, true, fast_load, state);
  }

  /**
   * Removes block and all its successors.
   * @param toRemove block to be removed.
   * @warning fails on assert if unknown hash is provided
   */
  void removeSubtree(index_t& toRemove) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("remove subtree %s", toRemove.toPrettyString());

    VBK_ASSERT_MSG(!toRemove.isRoot(), "cannot remove the root block");
    // save the pointer to the previous block
    auto* prev = toRemove.pprev;

    bool isOnMainChain = activeChain_.contains(&toRemove);
    if (isOnMainChain) {
      ValidationState dummy;
      bool success = this->setState(*prev, dummy);
      VBK_ASSERT_MSG(success, "err: %s", dummy.toString());
    }

    forEachNodePostorder<block_t>(
        toRemove,
        [&](index_t& next) {
          onBeforeLeafRemoved(next);
          tips_.erase(&next);
          VBK_ASSERT(!next.isDeleted());
          next.deleteTemporarily();
        },
        [&](index_t& next) { return !next.isDeleted(); });

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
    auto nondeletedDescendantCount = toRemove.nondeletedDescendantCount();

    VBK_ASSERT_MSG(nondeletedDescendantCount == 0,
                   "not a leaf block %s, has %d non-deleted descendants",
                   toRemove.toPrettyString(),
                   nondeletedDescendantCount);
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
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("Invalidating %s subtree: reason=%d block=%s",
                  block_t::name(),
                  (int)reason,
                  toBeInvalidated.toShortPrettyString());

    VBK_ASSERT_MSG(!toBeInvalidated.isRoot(),
                   "cannot invalidate the root block");

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

    // flag the child subtrees (excluding current block) as BLOCK_FAILED_CHILD
    for (auto* ptr : toBeInvalidated.pnext) {
      auto* pnext = ptr;
      forEachNodePreorder<block_t>(*pnext, [&](index_t& index) {
        bool failed = index.isFailed();
        doInvalidate(index, BLOCK_FAILED_CHILD);
        return !failed;
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
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("Revalidating %s subtree: reason=%d block=%s",
                  block_t::name(),
                  (int)reason,
                  toBeValidated.toShortPrettyString());

    VBK_ASSERT_MSG(!toBeValidated.isRoot(), "cannot revalidate the root block");

    VBK_ASSERT_MSG(isValidInvalidationReason(reason),
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
        bool failed = index.isFailed();
        return !failed;
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
    if (index == nullptr) {
      return state.Invalid(block_t::name() + "-setstate-unknown-block",
                           "could not find the block to set the state to");
    }
    return setState(*index, state);
  }

  virtual bool setState(index_t& index, ValidationState&) {
    VBK_TRACE_ZONE_SCOPED;
    overrideTip(index);
    return true;
  }

  virtual void overrideTip(index_t& to) {
    onBeforeOverrideTip.emit(to);
    activeChain_.setTip(&to);
    appliedBlockCount = activeChain_.blocksCount();
  }

  index_t& getRoot() const {
    VBK_ASSERT_MSG(isBootstrapped(), "must be bootstrapped");
    auto* root = getBestChain().first();
    VBK_ASSERT_MSG(root, "must be bootstrapped");
    return *root;
  }

  //! the number of blocks that have BLOCK_APPLIED flag set
  //! @private
  size_t appliedBlockCount = 0;

 protected:
  //! @private
  void finalizeBlocks(int32_t maxReorgBlocks,
                      int32_t preserveBlocksBehindFinal) {
    auto* tip = this->getBestChain().tip();
    VBK_ASSERT_MSG(tip, "%s tree must be bootstrapped", block_t::name());
    VBK_ASSERT(!this->isLoadingBlocks_);

    if (tip->getHeight() < maxReorgBlocks) {
      // skip finalization
      return;
    }

    int32_t firstBlockHeight = tip->getHeight() - maxReorgBlocks;
    int32_t bootstrapBlockHeight = this->getRoot().getHeight();
    firstBlockHeight = std::max(bootstrapBlockHeight, firstBlockHeight);
    auto* finalizedIndex = this->getBestChain()[firstBlockHeight];
    VBK_ASSERT(finalizedIndex != nullptr);
    this->finalizeBlockImpl(*finalizedIndex, preserveBlocksBehindFinal);
  }

  //! @private
  virtual void determineBestChain(index_t& candidate,
                                  ValidationState& state) = 0;

  //! @private
  void tryAddTip(index_t* index) {
    VBK_TRACE_ZONE_SCOPED;
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

  //! Permanently erases block from a block tree.
  //! @invariant should be the only function that deallocates blocks.
  //! @invariant erased block must be a tip.
  //! @invariant erased block must be "deleted".
  //! @invariant erased block must exist in block tree.
  void deallocateBlock(index_t& block) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_ASSERT_MSG(block.isDeleted(),
                   "cannot erase a block that is not deleted %s",
                   block.toPrettyString());
    VBK_ASSERT_MSG(
        block.isTip(),
        "cannot erase a block that has ancestors as that would split "
        "the tree: %s",
        block.toPrettyString());

    onBlockBeforeDeallocated.emit(block);
    auto shortHash = makePrevHash(block.getHash());
    auto count = blocks_.erase(shortHash);
    VBK_ASSERT_MSG(count == 1,
                   "state corruption: block %s is not in the store",
                   block.toPrettyString());
  }

  //! create a new block index
  //! @private
  index_t* createBlockIndex(const hash_t& hash, index_t& prev) {
    VBK_TRACE_ZONE_SCOPED;
    auto shortHash = makePrevHash(hash);
    auto newIndex = make_unique<index_t>(&prev);
    auto inserted = blocks_.emplace(shortHash, std::move(newIndex));
    VBK_ASSERT_MSG(inserted.second,
                   "attempted to create a blockindex with duplicate hash %s",
                   HexStr(shortHash));
    return inserted.first->second.get();
  }

  //! create a new block index with the empty prev field
  //! @private
  index_t* createBootstrapBlockIndex(const hash_t& hash,
                                     block_height_t height) {
    VBK_TRACE_ZONE_SCOPED;
    auto shortHash = makePrevHash(hash);
    auto newIndex = make_unique<index_t>(height);
    auto inserted = blocks_.emplace(shortHash, std::move(newIndex));
    VBK_ASSERT_MSG(inserted.second,
                   "attempted to create a blockindex with duplicate hash %s",
                   HexStr(shortHash));
    auto* index = inserted.first->second.get();
    // bootstrap blocks are finalized by default
    index->finalized = true;
    return index;
  }

  //! the block header is created in a deleted state
  //! @private
  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header,
                               block_height_t bootstrapHeight = 0) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_ASSERT(header != nullptr);

    auto* prev = getBlockIndex(header->getPreviousBlock());

    VBK_ASSERT_MSG(isBootstrapped() == (prev != nullptr),
                   "already bootstrapped");

    index_t* current =
        prev == nullptr
            ? createBootstrapBlockIndex(header->getHash(), bootstrapHeight)
            : createBlockIndex(header->getHash(), *prev);
    current->setHeader(std::move(header));

    return current;
  }

  //! @private
  index_t* insertBlockHeader(const std::shared_ptr<block_t>& block,
                             block_height_t bootstrapHeight = 0) {
    VBK_TRACE_ZONE_SCOPED;

    assertBlockSanity(*block);

    auto hash = block->getHash();
    index_t* current = findBlockIndex(hash);

    if (current != nullptr) {
      // it is a duplicate
      if (!current->isDeleted()) {
        return current;
      }

      VBK_ASSERT_MSG(*block == current->getHeader(),
                     "hash collision detected between %s and %s",
                     block->toPrettyString(),
                     current->getHeader().toPrettyString());
    } else {
      current = doInsertBlockHeader(block, bootstrapHeight);
    }

    current->restore();
    tryAddTip(current);

    this->onBlockInserted(current);

    // raise validity may return false if the block is invalid
    current->raiseValidity(BLOCK_VALID_TREE);
    return current;
  }

  //! @private
  bool loadBlockInner(const stored_index_t& index,
                      bool connectForward,
                      bool fast_load,
                      ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;

    VBK_ASSERT_MSG(isBootstrapped(), "should be bootstrapped");
    VBK_ASSERT_MSG(
        !this->isLoaded_, "%s tree must not be loaded", block_t::name());

    // indicate that we're in "loadTree" state.
    this->isLoadingBlocks_ = true;

    // quick check if given block is sane
    auto& root = getRoot();
    if (!fast_load) {
      if (connectForward) {
        if (index.height < root.getHeight()) {
          auto hash = index.header->getHash();
          return state.Invalid(
              "bad-height",
              format("Blocks can be forward connected after root only. "
                     "index.height: {}, root.height: {}, index.hash: {}",
                     index.height,
                     root.getHeight(),
                     HexStr(hash.begin(), hash.end())));
        }
      } else {
        if (index.height + 1 != root.getHeight()) {
          auto hash = index.header->getHash();
          return state.Invalid(
              "bad-height",
              format("Blocks can be backwards connected only to the "
                     "root, index.height: {}, root.height: {}, index.hash: {}",
                     index.height,
                     root.getHeight(),
                     HexStr(hash.begin(), hash.end())));
        }
      }
    }

    if (!fast_load && index.height == root.getHeight() &&
        index.header->getHash() != root.getHash()) {
      // root is finalized, we can't load a block on same height
      return state.Invalid("bad-root",
                           format("Can't overwrite root block with block {}",
                                  index.toPrettyString()));
    }

    auto& header = *index.header;
    auto currentHash = header.getHash();
    auto* current = findBlockIndex(currentHash);

    // we can not load a block, which already exists on chain and is not a
    // bootstrap block
    if (!fast_load && current != nullptr && !current->isDeleted() &&
        !current->hasFlags(BLOCK_BOOTSTRAP)) {
      return state.Invalid(
          "block-exists",
          "Found duplicate block, which is not bootstrap block");
    }

    if (current != nullptr) {
      if (current->isDeleted()) {
        current->restore();
      }
    } else {
      auto* prev = getBlockIndex(header.getPreviousBlock());
      // we can not load block which is not connected to the
      // bootstrap block
      if (prev == nullptr) {
        if (!fast_load) {
          if (connectForward) {
            return state.Invalid("bad-prev",
                                 "Block does not connect to current tree");
          }

          if (root.getHeader().getPreviousBlock() !=
              makePrevHash(currentHash)) {
            return state.Invalid(
                "bad-block-hash",
                format("Can't replace root block with block {} ",
                       index.toPrettyString()));
          }
        }

        current = createBootstrapBlockIndex(currentHash, index.height);
        current->restore();
        current->pnext.insert(&root);
        VBK_ASSERT(root.isRoot());
        root.pprev = current;

      } else {
        current = createBlockIndex(currentHash, *prev);
        current->restore();
      }
    }

    VBK_ASSERT(current);
    VBK_ASSERT(!current->isDeleted());

    // touchBlockIndex may return an existing block (one of bootstrap blocks),
    // so backup its 'pnext'
    auto next = current->pnext;
    auto prev = current->pprev;
    // copy all fields
    current->mergeFrom(index);
    // FIXME: this is an ugly HACK
    // clear inmem fields
    current->setNullInmemFields();
    // recover pnext
    current->pnext = next;
    // recover pprev
    current->pprev = prev;

    // check for valid previous block
    if (!fast_load && current->pprev != nullptr) {
      // prev block found
      auto expectedHeight = current->pprev->getHeight() + 1;
      if (current->getHeight() != expectedHeight) {
        return state.Invalid("bad-height");
      }
    }

    // set a new bootstrap block which is the prev block for the current
    // bootstrap block
    if ((current->pprev == nullptr) && (activeChain_.first() != current) &&
        !connectForward) {
      activeChain_.prependRoot(current);
    }

    VBK_ASSERT(!current->isDeleted());
    current->raiseValidity(BLOCK_VALID_TREE);
    current->unsetDirty();

    tryAddTip(current);

    return true;
  }

  std::string toPrettyString(size_t level = 0) const {
    auto tip = activeChain_.tip();
    std::string pad(level, ' ');

    std::string blocksStr{};
    // sort blocks by height
    std::vector<std::pair<int, index_t*>> byheight;
    byheight.reserve(blocks_.size());
    for (const auto& p : blocks_) {
      // FIXME: ugly hack due to trees being compared via toPrettyString in
      // tests
      if (!p.second->isDeleted()) {
        byheight.emplace_back(p.second->getHeight(), p.second.get());
      }
    }
    std::sort(byheight.rbegin(), byheight.rend());
    for (const auto& p : byheight) {
      blocksStr += (p.second->toPrettyString(level + 2) + "\n");
    }

    std::string tipsStr{};
    for (const auto* _tip : tips_) {
      tipsStr += (_tip->toPrettyString(level + 2) + "\n");
    }

    return format("{}{{tip={}}}\n{}{{blocks=\n{}{}}}\n{}{{tips=\n{}{}}}",
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
  const FinalizedPayloadsIndex<index_t>& getFinalizedPayloadsIndex() const {
    return finalizedPayloadsIndex_;
  }

  const PayloadsIndex<index_t>& getPayloadsIndex() const {
    return payloadsIndex_;
  }

  //! @private
  class DeferForkResolutionGuard {
    BaseBlockTree<Block>& tree_;

   public:
    DeferForkResolutionGuard(BaseBlockTree<Block>& tree) : tree_(tree) {
      tree_.deferForkResolution();
    }

    // non-copyable
    DeferForkResolutionGuard(const DeferForkResolutionGuard& other) = delete;
    DeferForkResolutionGuard& operator=(const DeferForkResolutionGuard& other) =
        delete;
    // movable
    DeferForkResolutionGuard(DeferForkResolutionGuard&& o) noexcept = default;
    DeferForkResolutionGuard& operator=(DeferForkResolutionGuard&& o) noexcept =
        default;

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
   * Deallocates a "connected" tree which contains block with hash=`hash`.
   *
   * @invariant to deallocate part of a tree, disconnect tree which must remain.
   *
   * @param block
   * @private
   */
  void deallocateTree(index_t& toDelete) {
    VBK_TRACE_ZONE_SCOPED;
    auto* index = &toDelete;

    // find oldest ancestor
    while (index != nullptr && !index->isRoot()) {
      index = index->pprev;
    }

    // starting at oldest ancestor, remove all blocks during post-order tree
    // traversal
    forEachNodePostorder<block_t>(
        *index,
        [&](index_t& next) {
          //! @warning: do not use logger in destructor, as logger instance may
          //! be deallocated!

          if (!next.isDeleted()) {
            onBeforeLeafRemoved(next);
            next.deleteTemporarily();
          }

          next.disconnectFromPrev();
          deallocateBlock(next);
        },
        [&](index_t&) { return true; });
  }

  inline void decreaseAppliedBlockCount(size_t erasedBlocks) {
    VBK_ASSERT_MSG(appliedBlockCount >= erasedBlocks,
                   "Tree: %s, Applied: %d, erased: %d",
                   block_t::name(),
                   appliedBlockCount,
                   erasedBlocks);
    appliedBlockCount -= erasedBlocks;
  }

  //! Marks `block` as finalized.
  //!
  //! Final blocks can not be reorganized, thus we can remove outdated blocks
  //! from tips_ and free some RAM by deallocating blocks past last final block
  //! minus `N` (`preserveBlocksBehindFinal`) blocks. In ALT and VBK chains N
  //! will equal to at least Endorsement Settlement Interval, so that
  //! endorsements in blocks after finalized block will point to an existing
  //! *endorsed* block.
  //!
  //! @param[in] block index to be finalized. Must be a part of active
  //! chain.
  //!
  //! @warning This action is irreversible. You will need to reload the whole
  //! tree to recover in case if this func is called by mistake.
  //!
  //! @private
  virtual void finalizeBlockImpl(index_t& index,
                                 // see config.preserveBlocksBehindFinal()
                                 int32_t preserveBlocksBehindFinal) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("Finalize %s, preserve %d blocks behind",
                  index.toShortPrettyString(),
                  preserveBlocksBehindFinal);
    VBK_ASSERT_MSG(appliedBlockCount == activeChain_.blocksCount(),
                   "Tree=%s Applied=%d active chain=%d",
                   block_t::name(),
                   appliedBlockCount,
                   activeChain_.blocksCount());

    index_t* finalizedBlock = &index;

    if (finalizedBlock == &this->getRoot()) {
      if (finalizedBlock->finalized) {
        // block is already finalized
        return;
      }

      finalizedBlock->finalized = true;
      finalizedPayloadsIndex_.addBlock(*finalizedBlock);
      return;
    }

    VBK_ASSERT(activeChain_.contains(finalizedBlock));

    // we need to clarify which block will be final. we can not remove blocks
    // which have not been saved into the storage
    for (auto* walkBlock = finalizedBlock; walkBlock != nullptr;
         walkBlock = walkBlock->pprev) {
      if (walkBlock->isDirty()) {
        finalizedBlock = walkBlock;
      }
    }

    // first, erase candidates from tips_ that will never be activated
    erase_if<decltype(tips_), index_t*>(
        tips_, [this, &finalizedBlock](index_t* tip) -> bool {
          VBK_ASSERT(tip);

          // tip from active chain can not be outdated
          if (!activeChain_.contains(tip) &&
              isBlockOutdated(*finalizedBlock, *tip)) {
            // we need to clarify which block will be final. we can not remove
            // blocks which have not been saved into the storage
            bool newFinalized = false;
            auto* walkBlock = tip;
            for (; tip != nullptr && !activeChain_.contains(walkBlock);
                 walkBlock = walkBlock->pprev) {
              if (walkBlock->isDirty()) {
                newFinalized = true;
              }
            }

            if (newFinalized) {
              finalizedBlock = walkBlock;
            }

            return true;
          }

          return false;
        });

    // second, update active chain (it should start with
    // 'finalizedBlock' but we also need to preserve `preserveBlocksBehindFinal`
    // blocks before it). all outdated blocks behind `finalizedBlock` block will
    // be deallocated
    int32_t firstBlockHeight =
        finalizedBlock->getHeight() - preserveBlocksBehindFinal;
    int32_t rootBlockHeight = getRoot().getHeight();
    firstBlockHeight = std::max(rootBlockHeight, firstBlockHeight);

    // before we deallocate subtree, disconnect "new root block" from previous
    // tree
    auto* newRoot = activeChain_[firstBlockHeight];
    VBK_ASSERT(newRoot);

    auto* rootPrev = newRoot->pprev;
    if (newRoot->pprev != nullptr) {
      newRoot->pprev->pnext.erase(newRoot);
      newRoot->pprev = nullptr;

      // do deallocate
      deallocateTree(*rootPrev);

      // erase "parallel" blocks - blocks that are on same height as `index`,
      // but since `index` is final, will never be active.
      if (finalizedBlock->pprev != nullptr) {
        auto parallelBlocks = finalizedBlock->pprev->pnext;
        parallelBlocks.erase(finalizedBlock);
        for (auto* par : parallelBlocks) {
          // disconnect `par` from prev block
          par->disconnectFromPrev();
          deallocateTree(*par);
        }
      }
    }

    // update active chain
    VBK_ASSERT(firstBlockHeight >= rootBlockHeight);
    size_t deallocatedBlocks = firstBlockHeight - rootBlockHeight;
    if (deallocatedBlocks > 0) {
      VBK_LOG_WARN(
          "%s tree deallocated blocks: %d", block_t::name(), deallocatedBlocks);
    }
    activeChain_ = Chain<index_t>(firstBlockHeight, activeChain_.tip());
    appliedBlockCount = activeChain_.blocksCount();

    // fourth, mark `index` and all predecessors as finalized
    index_t* ptr = finalizedBlock;
    while (ptr != nullptr && !ptr->finalized) {
      ptr->finalized = true;
      finalizedPayloadsIndex_.addBlock(*ptr);
      ptr = ptr->pprev;
    }
  }

  //! callback which is executed when new block is added to a tree
  //! @private
  virtual void onBlockInserted(index_t* /*ignore*/) {
    /* do nothing in base tree */
  }

  /**
   * Find all tips affected by a block modification and schedule or do fork
   * resolution
   * @private
   */
  void updateAffectedTips(index_t& modifiedBlock) {
    VBK_TRACE_ZONE_SCOPED;
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
   *
   * @private
   */
  void doUpdateAffectedTips(index_t& modifiedBlock, ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;
    auto tips = findValidTips<block_t>(this->getTips(), modifiedBlock);
    VBK_LOG_DEBUG(
        "Found %d affected valid tips in %s", tips.size(), block_t::name());
    for (auto* tip : tips) {
      determineBestChain(*tip, state);
    }
  }

  //! @private
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
    VBK_TRACE_ZONE_SCOPED;
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
    VBK_ASSERT_MSG(
        success,
        "state corruption: could not revert to the saved best chain tip: %s",
        dummy.toString());
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

  //! callback which is executed when block is removing from the tree
  //! @private
  virtual void onBeforeLeafRemoved(const index_t&) {}

  void doInvalidate(index_t& block, enum BlockValidityStatus reason) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_ASSERT_MSG(
        !block.isValidUpTo(BLOCK_CAN_BE_APPLIED) ||
            (reason & BLOCK_FAILED_POP) == 0u,
        "attempted to set mutually exclusive flags BLOCK_CAN_BE_APPLIED "
        "and BLOCK_FAILED_POP for block %s",
        block.toPrettyString());

    block.setFlag(reason);
    tips_.erase(&block);

    onBlockValidityChanged.emit(block);
  }

  void doReValidate(index_t& block, enum BlockValidityStatus reason) {
    block.unsetFlag(reason);
    tryAddTip(&block);
    onBlockValidityChanged.emit(block);
  }

 public:
  //! before we deallocate any block index we emit it to this signal
  signals::Signal<void(const index_t&)> onBlockBeforeDeallocated;
  //! signals to the end user that block have been invalidated
  signals::Signal<on_invalidate_t> onBlockValidityChanged;
  //! chain reorg signal - the tip is being changed
  signals::Signal<void(const index_t& index)> onBeforeOverrideTip;

 protected:
  //! if true, we're in "loading blocks" state
  bool isLoadingBlocks_ = false;
  //! if true, we can no longer execute loadBlock/loadTip on this tree.
  bool isLoaded_ = false;
  //! stores ALL blocks, including valid and invalid
  block_index_t blocks_;
  //! stores ONLY VALID tips, including currently active tip
  std::unordered_set<index_t*> tips_;
  //! currently applied chain
  Chain<index_t> activeChain_;
  //! stores mapping of payload id -> its containing ALT/VBK block which are
  //! already finalized. For BTC tree does nothing. stores payloads only from
  //! finalized blocks.
  FinalizedPayloadsIndex<index_t> finalizedPayloadsIndex_;
  //! stores mapping of payload id -> its containing ALT/VBK blocks which are
  //! not yet finalized. For BTC tree does nothing.
  PayloadsIndex<index_t> payloadsIndex_;

  const BlockReader& blockProvider_;
};

}  // namespace altintegration
#endif  // ALTINTEGRATION_BASE_BLOCK_TREE_HPP
