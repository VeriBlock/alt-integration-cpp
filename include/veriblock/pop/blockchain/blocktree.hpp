// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <algorithm>
#include <memory>
#include <unordered_map>

#include "base_block_tree.hpp"
#include "block_index.hpp"
#include "blockchain_util.hpp"
#include "chain.hpp"
#include "tree_algo.hpp"
#include "veriblock/pop/fmt.hpp"
#include "veriblock/pop/stateless_validation.hpp"
#include "veriblock/pop/storage/block_reader.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {

/**
 * BlockTree is a tree of blocks with single "bootstrap" block as root.
 * @tparam Block
 */
template <typename Block, typename ChainParams>
struct BlockTree : public BaseBlockTree<Block> {
  using base = BaseBlockTree<Block>;
  using block_t = Block;
  using params_t = ChainParams;
  using index_t = typename base::index_t;
  using stored_index_t = typename base::stored_index_t;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = typename Block::prev_hash_t;
  using height_t = typename Block::height_t;

  ~BlockTree() override = default;

  BlockTree(const ChainParams& param, const BlockReader& blockProvider)
      : base(blockProvider), param_(&param) {}

  const ChainParams& getParams() const { return *param_; }

  /**
   * Bootstrap blockchain with a single genesis block.
   */
  virtual void bootstrapWithGenesis(const block_t& block) {
    VBK_ASSERT(!base::isBootstrapped() && "already bootstrapped");
    VBK_ASSERT(!this->isLoaded_);
    VBK_ASSERT(!this->isLoadingBlocks_);
    ValidationState state;
    this->bootstrap(0, block, state);
    this->isLoadingBlocks_ = false;
  }

  /**
   * Bootstrap network with a chain that starts at 'startHeight'
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @param[in] startHeight start height of the chain
   * @param[in] chain bootstrap chain
   */
  virtual void bootstrapWithChain(height_t startHeight,
                                  const std::vector<block_t>& chain) {
    VBK_ASSERT(!base::isBootstrapped() && "already bootstrapped");
    VBK_ASSERT(!this->isLoadingBlocks_);
    VBK_ASSERT(!this->isLoaded_);
    VBK_ASSERT_MSG(!chain.empty(), "bootstrap chain must not be empty");
    VBK_ASSERT_MSG(
        chain.size() >= param_->numBlocksForBootstrap(),
        "bootstrap chain is too small (%d) expected at least %d blocks",
        chain.size(),
        param_->numBlocksForBootstrap());

    ValidationState state;

    // pick first block from the chain, bootstrap with a single block
    auto genesis = chain[0];
    this->bootstrap(startHeight, genesis, state);

    // apply the rest of the blocks from the chain on top of our bootstrap
    // block. disable difficulty checks, because we have not enough blocks in
    // our store (yet) to check it correctly
    for (size_t i = 1, size = chain.size(); i < size; i++) {
      auto& block = chain[i];
      bool ok = this->acceptBlockHeaderImpl(
          std::make_shared<block_t>(block), state, false);
      VBK_ASSERT_MSG(ok,
                     "found statelssly invalid bootstrap block: %d, %s",
                     i,
                     state.toString());
      auto* index = base::getBlockIndex(block.getHash());
      VBK_ASSERT(index);
      index->setIsBootstrap(true);
    }
  }

  bool acceptBlockHeader(const block_t& block, ValidationState& state) {
    return acceptBlockHeaderImpl(std::make_shared<block_t>(block), state, true);
  }

  bool acceptBlockHeader(const std::shared_ptr<block_t>& block,
                         ValidationState& state) {
    return acceptBlockHeaderImpl(block, state, true);
  }

  std::string toPrettyString(size_t level = 0) const {
    std::string pad(level, ' ');
    return format("{}{}BlockTree{{blocks={}\n{}\n{}}}",
                  pad,
                  Block::name(),
                  base::getBlocks().size(),
                  base::toPrettyString(level + 2),
                  pad);
  }

  //! @invariant NOT atomic.
  bool loadBlockForward(const stored_index_t& index,
                        bool fast_load,
                        ValidationState& state) override {
    if (!fast_load && !checkBlock(*index.header, state, *param_)) {
      return state.Invalid("bad-header");
    }

    const auto hash = index.header->getHash();
    if (!base::loadBlockForward(index, fast_load, state)) {
      return false;
    }

    auto* current = base::getBlockIndex(hash);
    VBK_ASSERT(current);
    VBK_ASSERT(!this->isLoaded_);
    VBK_ASSERT(this->isLoadingBlocks_);

    // we only check blocks contextually if they are not bootstrap blocks, and
    // previous block exists
    if (!fast_load && !current->isRoot() &&
        !current->hasFlags(BLOCK_BOOTSTRAP) &&
        !contextuallyCheckBlock(
            *current->pprev, current->getHeader(), state, *param_)) {
      return state.Invalid("bad-block-contextually");
    }

    // recover chainwork
    current->chainWork = getBlockProof(current->getHeader());
    if (!current->isRoot()) {
      current->chainWork += current->pprev->chainWork;
    }

    // clear blockOfProofEndorsements inmem field
    current->clearBlockOfProofEndorsement();

    current->raiseValidity(BLOCK_VALID_TREE);
    return true;
  }

  //! block is considered old if it is behind current tip further than 'old
  //! blocks window' blocks
  bool isBlockOld(height_t height) const {
    auto* tip = base::getBestChain().tip();
    VBK_ASSERT(tip);

    return tip->getHeight() - height > getParams().getOldBlocksWindow();
  }

  //! @overload
  bool isBlockOld(const hash_t& hash) const {
    auto* index = base::getBlockIndex(hash);
    if (index == nullptr) {
      // block is unknown, so not "old"
      return false;
    }

    return isBlockOld(index->getHeight());
  }

  void finalizeBlocks() {
    VBK_ASSERT(!this->isLoadingBlocks_);
    base::finalizeBlocks(this->getParams().getMaxReorgBlocks(),
                         this->getParams().preserveBlocksBehindFinal());
  }

 protected:
  const ChainParams* param_ = nullptr;

  bool acceptBlockHeaderImpl(const std::shared_ptr<block_t>& block,
                             ValidationState& state,
                             bool shouldContextuallyCheck) {
    index_t* index = nullptr;
    if (!validateAndAddBlock(block, state, shouldContextuallyCheck, &index)) {
      return false;
    }

    VBK_ASSERT(index);
    if (!shouldContextuallyCheck) {
      // this is a bootstrap block
      index->setFlag(BLOCK_BOOTSTRAP);
    }

    base::tryAddTip(index);

    // don't defer fork resolution in the acceptBlockHeader+addPayloads flow
    // until the validation hole is plugged
    determineBestChain(*index, state);

    return true;
  }

  void bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    bootstrap(height, std::make_shared<block_t>(block), state);
  }

  void bootstrap(height_t height,
                 std::shared_ptr<block_t> block,
                 ValidationState& state) {
    bool ok = checkBlock(*block, state, *param_);
    VBK_ASSERT_MSG(ok,
                   "found statelessly invalid bootstrap block in %s tree: %s",
                   block_t::name(),
                   state.toString());

    auto* index = base::insertBlockHeader(block, height);
    VBK_ASSERT(index != nullptr &&
               "insertBlockHeader should have never returned nullptr");

    index->setHeight(height);
    base::activeChain_ = Chain<index_t>(height, index);
    index->setIsBootstrap(true);

    VBK_ASSERT(index->finalized);
    VBK_ASSERT(base::isBootstrapped());
    VBK_ASSERT(base::getBlockIndex(index->getHash()) != nullptr &&
               "getBlockIndex must be able to find the block added by "
               "insertBlockHeader");

    base::tryAddTip(index);
    index->setFlag(BLOCK_ACTIVE);
    bool success = index->raiseValidity(BLOCK_CAN_BE_APPLIED);
    VBK_ASSERT(success);
    index->setFlag(BLOCK_BOOTSTRAP);
    base::appliedBlockCount = 1;
  }

  bool validateAndAddBlock(const std::shared_ptr<block_t>& block,
                           ValidationState& state,
                           bool shouldContextuallyCheck,
                           index_t** ret) {
    if (!checkBlock(*block, state, *param_)) {
      return state.Invalid(block_t::name() + "-check-block");
    }

    // we must know previous block
    auto* prev = base::getBlockIndex(block->getPreviousBlock());
    if (prev == nullptr) {
      return state.Invalid(
          block_t::name() + "-bad-prev-block",
          "can not find previous block: " + HexStr(block->getPreviousBlock()));
    }

    if (shouldContextuallyCheck &&
        !contextuallyCheckBlock(*prev, *block, state, *param_)) {
      return state.Invalid(block_t::name() + "-contextually-check-block");
    }

    auto index = base::insertBlockHeader(block);
    VBK_ASSERT(index != nullptr &&
               "insertBlockHeader should have never returned nullptr");

    if (ret) {
      *ret = index;
    }

    index->raiseValidity(BLOCK_CONNECTED);

    // if prev block is invalid, mark this block as invalid
    if (!prev->isValid()) {
      index->setFlag(BLOCK_FAILED_CHILD);
      return state.Invalid(
          block_t::name() + "-bad-chain",
          format("Previous block is invalid={}", prev->toPrettyString()));
    }

    return true;
  }

  void determineBestChain(index_t& candidate, ValidationState& state) override {
    auto bestTip = base::getBestChain().tip();
    VBK_ASSERT(bestTip != nullptr && "must be bootstrapped");

    if (bestTip == &candidate) {
      return;
    }

    // do not even consider invalid candidates
    if (!candidate.isValid()) {
      return;
    }

    if (bestTip->chainWork < candidate.chainWork) {
      //! important to use this->setState for proper vtable resolution
      this->setState(candidate, state);
    }
  }

  //! whenever new block is inserted, BlockTree has to update its ChainWork
  void onBlockInserted(index_t* newIndex) final {
    newIndex->chainWork = getBlockProof(newIndex->getHeader());
    if (!newIndex->isRoot()) {
      newIndex->chainWork += newIndex->pprev->chainWork;
    }
  }
};

//! @private
template <typename Block, typename ChainParams>
void PrintTo(const BlockTree<Block, ChainParams>& tree, std::ostream* os) {
  *os << tree.toPrettyString();
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
