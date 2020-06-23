// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/base_block_tree.hpp>
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/tree_algo.hpp>
#include <veriblock/context.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/validation_state.hpp>

#include "veriblock/fmt.hpp"

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
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = decltype(Block::previousBlock);
  using height_t = typename Block::height_t;

  ~BlockTree() override = default;

  BlockTree(const ChainParams& param) : param_(&param) {}

  const ChainParams& getParams() const { return *param_; }

  /**
   * Bootstrap blockchain with a single genesis block, from "chain parameters"
   * passed in constructor.
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @return true if bootstrap was successful, false otherwise
   */
  virtual bool bootstrapWithGenesis(ValidationState& state) {
    VBK_ASSERT(base::blocks_.empty() && "already bootstrapped");
    auto block = param_->getGenesisBlock();
    return this->bootstrap(0, block, state);
  }

  /**
   * Bootstrap network with a chain that starts at 'startHeight'
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @param startHeight start height of the chain
   * @param chain bootstrap chain
   * @return true if bootstrap was successful, false otherwise
   */
  virtual bool bootstrapWithChain(height_t startHeight,
                                  const std::vector<block_t>& chain,
                                  ValidationState& state) {
    VBK_ASSERT(base::blocks_.empty() && "already bootstrapped");
    if (chain.empty()) {
      return state.Invalid("bootstrap-empty-chain",
                           "provided bootstrap chain is empty");
    }

    if (chain.size() < param_->numBlocksForBootstrap()) {
      return state.Invalid(
          "bootstrap-small-chain",
          "number of blocks in the provided chain is too small: " +
              std::to_string(chain.size()) + ", expected at least " +
              std::to_string(param_->numBlocksForBootstrap()));
    }

    // pick first block from the chain, bootstrap with a single block
    auto genesis = chain[0];
    if (!this->bootstrap(startHeight, genesis, state)) {
      return state.Invalid("blocktree-bootstrap");
    }

    // apply the rest of the blocks from the chain on top of our bootstrap
    // block. disable difficulty checks, because we have not enough blocks in
    // our store (yet) to check it correctly
    for (size_t i = 1, size = chain.size(); i < size; i++) {
      auto& block = chain[i];
      if (!this->acceptBlock(std::make_shared<block_t>(block), state, false)) {
        return state.Invalid("blocktree-accept");
      }
    }

    return true;
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    return acceptBlock(std::make_shared<block_t>(block), state, true);
  }

  bool acceptBlock(const std::shared_ptr<block_t>& block,
                   ValidationState& state) {
    return acceptBlock(block, state, true);
  }

  std::string toPrettyString(size_t level = 0) const {
    std::string pad(level, ' ');
    return fmt::sprintf("%s%sBlockTree{blocks=%llu\n%s\n%s}",
                        pad,
                        Block::name(),
                        base::blocks_.size(),
                        base::toPrettyString(level + 2),
                        pad);
  }

 protected:
  const ChainParams* param_ = nullptr;

  bool acceptBlock(const std::shared_ptr<block_t>& block,
                   ValidationState& state,
                   bool shouldContextuallyCheck) {
    index_t* index = nullptr;
    if (!validateAndAddBlock(block, state, shouldContextuallyCheck, &index)) {
      VBK_LOG_WARN("Found invalid block %s %s",
                   state.toString(),
                   block->toPrettyString());
      return false;
    }

    bool isBootstrap = !shouldContextuallyCheck;
    determineBestChain(base::activeChain_, *index, state, isBootstrap);

    base::tryAddTip(index);

    return true;
  }

  bool bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    return bootstrap(height, std::make_shared<block_t>(block), state);
  }

  bool bootstrap(height_t height,
                 const std::shared_ptr<block_t>& block,
                 ValidationState& state) {
    if (!checkBlock(*block, state, *param_)) {
      return state.Invalid("bootstrap");
    }

    auto* index = base::insertBlockHeader(block);
    index->height = height;

    base::activeChain_ = Chain<index_t>(height, index);

    if (!base::blocks_.empty() && !base::getBlockIndex(block->getHash())) {
      return state.Error("block-index-no-genesis");
    }

    return this->setTip(*index, state, true);
  }

  bool validateAndAddBlock(const std::shared_ptr<block_t>& block,
                           ValidationState& state,
                           bool shouldContextuallyCheck,
                           index_t** ret) {
    if (!checkBlock(*block, state, *param_)) {
      return state.Invalid(block_t::name() + "-check-block");
    }

    // we must know previous block
    auto* prev = base::getBlockIndex(block->previousBlock);
    if (prev == nullptr) {
      return state.Invalid(
          block_t::name() + "-bad-prev-block",
          "can not find previous block: " + HexStr(block->previousBlock));
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

    // if prev block is invalid, mark this block as invalid
    if (!prev->isValid()) {
      index->setFlag(BLOCK_FAILED_CHILD);
      return state.Invalid(
          block_t::name() + "-bad-chain",
          fmt::sprintf("Previous block is invalid=%s", prev->toPrettyString()));
    }

    return true;
  }

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          ValidationState& state,
                          bool isBootstrap = false) override {
    if (VBK_UNLIKELY(IsShutdownRequested())) {
      return;
    }

    if (currentBest.tip() == &indexNew) {
      return;
    }

    // do not even consider invalid indices
    if (!indexNew.isValid()) {
      return;
    }

    auto* prev = currentBest.tip();
    if (prev == nullptr || prev->chainWork < indexNew.chainWork) {
      VBK_LOG_DEBUG("Doing POW fork resolution Active=%s, Candidate=%s",
                    (prev ? prev->toPrettyString() : "<nullptr>"),
                    indexNew.toPrettyString());
      //! important to use this->setTip for proper vtable resolution
      this->setTip(indexNew, state, isBootstrap);
    }
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
