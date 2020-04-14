#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

/**
 * BlockTree is a tree of blocks with single "bootstrap" block as root.
 * @tparam Block
 */
template <typename Block, typename ChainParams>
struct BlockTree {
  using block_t = Block;
  using params_t = ChainParams;
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = decltype(Block::previousBlock);
  using height_t = typename Block::height_t;
  using payloads_t = typename block_t::payloads_t;
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::shared_ptr<index_t>>;

  virtual ~BlockTree() = default;

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
    assert(block_index_.empty() && "already bootstrapped");
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
    assert(block_index_.empty() && "already bootstrapped");
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
      if (!this->acceptBlock(block, state, false)) {
        return state.Invalid("blocktree-accept");
      }
    }

    return true;
  }

  template <size_t N,
            typename = typename std::enable_if<N == prev_block_hash_t::size() ||
                                               N == hash_t::size()>::type>
  index_t* getBlockIndex(const Blob<N>& hash) const {
    auto shortHash = hash.template trimLE<prev_block_hash_t::size()>();
    auto it = block_index_.find(shortHash);
    return it == block_index_.end() ? nullptr : it->second.get();
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    return acceptBlock(block, state, true);
  }

  void invalidateTip() {
    auto* tip = getBestChain().tip();
    assert(tip != nullptr && "not bootstrapped...");
    invalidateBlockByHash(tip->getHash());
  }

  virtual void invalidateBlockByIndex(const index_t* blockIndex) {
    if (blockIndex == nullptr) {
      // no such block
      return;
    }

    std::vector<index_t*> fork_tips;

    for (auto chain_it = fork_chains_.begin();
         chain_it != fork_chains_.end();) {
      invalidateBlockFromChain(chain_it->second, blockIndex);

      if (chain_it->second.tip() == nullptr) {
        chain_it = fork_chains_.erase(chain_it);
        continue;
      }
      fork_tips.push_back(chain_it->second.tip());
      ++chain_it;
    }

    invalidateBlockFromChain(activeChain_, blockIndex);

    for (const auto& fork_tip : fork_tips) {
      determineBestChain(activeChain_, *fork_tip);
    }

    block_index_.erase(
        blockIndex->getHash().template trimLE<prev_block_hash_t::size()>());
  }

  virtual void invalidateBlockByHash(const hash_t& blockHash) {
    index_t* blockIndex = getBlockIndex(blockHash);
    invalidateBlockByIndex(blockIndex);
  }

  const Chain<index_t>& getBestChain() const { return this->activeChain_; }

  const block_index_t& getAllBlocks() const { return block_index_; }

 protected:
  block_index_t block_index_;

  // first we have to analyze the highest forks, to avoid the removing blocks
  // that can be proceed by another fork chain like in the situation described
  // below
  // We have two fork chains that one of the fork chain has been forked from
  // another
  //  F - G - H - I
  //      \ Q
  // and if block F is being removed, and chain 'F - G - H - I' will be
  // processed firstly, we will remove block 'G' which is needed for the
  // another chain, so that chain must be processed firstly

  std::multimap<typename Block::height_t,
                Chain<index_t>,
                std::greater<typename Block::height_t>>
      fork_chains_;
  Chain<index_t> activeChain_;

  // to make BlockTree copyable, copy-initializable, we need to make it ptr
  const ChainParams* param_ = nullptr;

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& fullHash) {
    auto hash = fullHash.template trimLE<prev_block_hash_t::size()>();
    auto it = block_index_.find(hash);
    if (it != block_index_.end()) {
      return it->second.get();
    }

    auto newIndex = std::make_shared<index_t>();
    it = block_index_.insert({hash, std::move(newIndex)}).first;
    return it->second.get();
  }

  index_t* insertBlockHeader(const block_t& block) {
    auto hash = block.getHash();
    index_t* current = getBlockIndex(hash);
    if (current) {
      // it is a duplicate
      return current;
    }

    current = touchBlockIndex(hash);
    current->header = std::make_shared<block_t>(block);
    current->pprev = getBlockIndex(block.previousBlock);

    if (current->pprev) {
      // prev block found
      current->height = current->pprev->height + 1;
      current->chainWork = current->pprev->chainWork + getBlockProof(block);
    } else {
      current->height = 0;
      current->chainWork = getBlockProof(block);
    }
    return current;
  }

  void invalidateBlockFromChain(Chain<index_t>& chain, const index_t* block) {
    if (block == nullptr) {
      return;
    }

    if (!chain.contains(block) &&
        !(activeChain_.contains(block) &&
          block->height < chain.first()->height) &&
        chain.first()->getAncestor(block->height) != block) {
      return;
    }

    while (chain.tip() != nullptr && chain.tip() != block) {
      disconnectTipFromChain(chain);
    }

    // disconnect the block from the current chain, but do not remove it from
    // block_index_
    if (chain.tip() != nullptr) {
      chain.disconnectTip();
    }
  }

  void disconnectTipFromChain(Chain<index_t>& chain) {
    BlockIndex<Block>* currentTip = chain.tip();
    hash_t tipHash = currentTip->getHash();

    chain.disconnectTip();
    block_index_.erase(tipHash.template trimLE<prev_block_hash_t::size()>());
  }

  void addForkCandidate(BlockIndex<Block>* newCandidate,
                        BlockIndex<Block>* oldCandidate) {
    if (newCandidate == nullptr) {
      return;
    }

    bool isAdded = false;
    auto forkChainStart = fork_chains_.end();
    for (auto chain_it = fork_chains_.begin();
         chain_it != fork_chains_.end();) {
      if (chain_it->second.tip() == newCandidate->pprev) {
        chain_it->second.setTip(newCandidate);
        isAdded = true;
      }

      if (chain_it->second.contains(newCandidate)) {
        isAdded = true;
      }

      if (chain_it->second.contains(newCandidate->pprev)) {
        forkChainStart = chain_it;
      }

      if (chain_it->second.tip() == oldCandidate ||
          (oldCandidate != nullptr &&
           chain_it->second.tip() == oldCandidate->pprev)) {
        chain_it = fork_chains_.erase(chain_it);
        continue;
      }
      ++chain_it;
    }

    if (activeChain_.contains(newCandidate) || isAdded) {
      return;
    }

    // find block from the main chain
    index_t* workBlock = newCandidate;
    if (forkChainStart == fork_chains_.end()) {
      for (; !activeChain_.contains(workBlock->pprev);
           workBlock = workBlock->pprev)
        ;
    }

    Chain<index_t> newForkChain(workBlock->height, workBlock);
    newForkChain.setTip(newCandidate);
    fork_chains_.insert(std::pair<typename Block::height_t, Chain<index_t>>(
        newForkChain.getStartHeight(), newForkChain));
  }

  bool acceptBlock(const block_t& block,
                   ValidationState& state,
                   bool shouldContextuallyCheck) {
    index_t* index = nullptr;
    if (!validateAndAddBlock(block, state, shouldContextuallyCheck, &index)) {
      return false;
    }

    bool isBootstrap = !shouldContextuallyCheck;
    determineBestChain(activeChain_, *index, isBootstrap);

    return true;
  }

  bool bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    if (!checkBlock(block, state, *param_)) {
      return state.Invalid("bootstrap");
    }

    auto* index = insertBlockHeader(block);
    index->height = height;

    activeChain_ = Chain<index_t>(height, index);

    if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
      return state.Error("block-index-no-genesis");
    }

    return true;
  }

  bool validateAndAddBlock(const block_t& block,
                           ValidationState& state,
                           bool shouldContextuallyCheck,
                           index_t** ret) {
    if (!checkBlock(block, state, *param_)) {
      return state.Invalid("check-block");
    }

    // we must know previous block
    auto* prev = getBlockIndex(block.previousBlock);
    if (prev == nullptr) {
      return state.Invalid("bad-prev-block", "can not find previous block");
    }

    if (shouldContextuallyCheck &&
        !contextuallyCheckBlock(*prev, block, state, *param_)) {
      return state.Invalid("contextually-check-block");
    }

    auto index = insertBlockHeader(block);
    assert(index != nullptr &&
           "insertBlockHeader should have never returned nullptr");

    if (ret) {
      *ret = index;
    }

    return true;
  }

  virtual void determineBestChain(Chain<index_t>& currentBest,
                                  index_t& indexNew,
                                  bool isBootstrap = false) {
    if (currentBest.tip() == &indexNew) {
      return;
    }

    if (currentBest.tip() == nullptr ||
        currentBest.tip()->chainWork < indexNew.chainWork) {
      auto prevTip = currentBest.tip();
      currentBest.setTip(&indexNew);
      onTipChanged(indexNew, isBootstrap);
      addForkCandidate(prevTip, &indexNew);
    } else {
      addForkCandidate(&indexNew, indexNew.pprev);
    }
  }

  //! callback, executed every time when tip is changed. Useful for derived
  //! classes.
  virtual void onTipChanged(index_t&, bool isBootstrap) { (void)isBootstrap; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
