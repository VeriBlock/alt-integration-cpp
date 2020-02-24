#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <memory>
#include <set>
#include <unordered_map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/fmt.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

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

  virtual ~BlockTree() = default;

  BlockTree(std::shared_ptr<BlockRepository<index_t>> repo,
            std::shared_ptr<ChainParams> param)
      : repo_(std::move(repo)), param_(std::move(param)) {}

  /**
   * Bootstrap blockchain with a single genesis block, from "chain parameters"
   * passed in constructor.
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @return true if bootstrap was successful, false otherwise
   */
  bool bootstrapWithGenesis(ValidationState& state) {
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
  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) {
    assert(block_index_.empty() && "already bootstrapped");
    if (chain.empty()) {
      return state.Invalid("bootstrapWithChain()",
                           "bootstrap-empty-chain",
                           "provided bootstrap chain is empty");
    }

    if (chain.size() < param_->numBlocksForBootstrap()) {
      return state.Invalid("bootstrapWithChain()",
                           "bootstrap-small-chain",
                           format("number of blocks in the provided chain is "
                                  "too small: %d, expected at least %d",
                                  chain.size(),
                                  param_->numBlocksForBootstrap()));
    }

    // pick first block from the chain, bootstrap with a single block
    auto genesis = chain[0];
    if (!this->bootstrap(startHeight, genesis, state)) {
      return state.addStackFunction("bootstrapWithChain()");
    }

    // apply the rest of the blocks from the chain on top of our bootstrap
    // block. disable difficulty checks, because we have not enough blocks in
    // our store (yet) to check it correctly
    for (size_t i = 1, size = chain.size(); i < size; i++) {
      auto& block = chain[i];
      if (!this->acceptBlock(block, state, false)) {
        return state.addStackFunction("bootstrapWithChain()");
      }
    }

    return true;
  }

  template <size_t N,
            typename = typename std::enable_if<N == prev_block_hash_t::size() ||
                                               N == hash_t::size()>::type>
  index_t* getBlockIndex(const Blob<N>& hash) {
    auto shortHash = hash.template trimLE<prev_block_hash_t::size()>();
    auto it = block_index_.find(shortHash);
    return it == block_index_.end() ? nullptr : it->second.get();
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    return acceptBlock(block, state, true);
  }

  const Chain<Block>& getBestChain() const { return this->activeChain_; }

 protected:
  std::unordered_map<prev_block_hash_t, std::unique_ptr<index_t>> block_index_;
  Chain<Block> activeChain_;
  std::shared_ptr<BlockRepository<index_t>> repo_;
  std::shared_ptr<ChainParams> param_;

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& fullHash) {
    auto hash = fullHash.template trimLE<prev_block_hash_t::size()>();
    auto it = block_index_.find(hash);
    if (it != block_index_.end()) {
      return it->second.get();
    }

    auto* newIndex = new index_t{};
    it = block_index_.insert({hash, std::unique_ptr<index_t>(newIndex)}).first;
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
    current->header = block;
    current->pprev = getBlockIndex(block.previousBlock);

    if (current->pprev) {
      // prev block found
      current->height = current->pprev->height + 1;
      current->chainWork = current->pprev->chainWork + getBlockProof(block);
    } else {
      current->height = 0;
      current->chainWork = getBlockProof(block);
    }

    determineBestChain(activeChain_, *current);

    return current;
  }

  bool load(ValidationState& state, bool bootstrapGenesis) {
    // at this point we should have bootstrap block(s) in our storage
    auto cursor = repo_->newCursor();

    // load blocks into memory
    std::vector<std::pair<height_t, std::unique_ptr<index_t>>> blocks;
    for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
      auto value = cursor->value();

      if (!checkProofOfWork(value.header, *param_)) {
        return state.Error("load-bad-proof-of-work");
      }

      auto index = std::unique_ptr<index_t>(new index_t{});
      *index = value;
      blocks.push_back({value.height, std::move(index)});
    }

    size_t processedBlocks = 0;
    size_t numBlocksForBootstrap = param_->numBlocksForBootstrap();
    assert(block_index_.size() == 1);
    std::sort(blocks.begin(), blocks.end());
    for (const auto& item : blocks) {
      index_t* index = item.second.get();
      bool checkDifficulty =
          bootstrapGenesis || processedBlocks++ > numBlocksForBootstrap;
      if (acceptBlock(index->header, state, checkDifficulty)) {
        return state.addStackFunction("load()");
      }
    }

    return true;
  }

  bool acceptBlock(const block_t& block,
                   ValidationState& state,
                   bool checkDifficulty) {
    if (!checkBlock(block, state, *param_)) {
      return state.addStackFunction("acceptBlockHeader()");
    }

    // we must know previous block
    auto* prev = getBlockIndex(block.previousBlock);
    if (prev == nullptr) {
      return state.Invalid("acceptBlockHeader()",
                           "bad-prev-block",
                           "can not find previous block");
    }

    // check difficulty
    if (checkDifficulty &&
        block.getDifficulty() != getNextWorkRequired(*prev, block, *param_)) {
      return state.Invalid(
          "acceptBlockHeader()", "bad-diffbits", "incorrect proof of work");
    }

    // TODO move this validation into the statefull function
    // ContextualCheckBlock() This validation that was moved from the btc, fails
    // with the vbk chains
    /*if (int64_t(block.getBlockTime()) < prev->getMedianTimePast()) {
      return state.Invalid("acceptBlockHeader()",
                           "time-too-old",
                           "block's timestamp is too early");
    }*/

    if (int64_t(block.getBlockTime()) >
        currentTimestamp4() + ALT_MAX_FUTURE_BLOCK_TIME) {
      return state.Invalid("acceptBlockHeader()",
                           "time-too-new",
                           "block timestamp too far in the future");
    }

    auto* index = insertBlockHeader(block);
    assert(index != nullptr &&
           "insertBlockHeader should have never returned nullptr");
    repo_->put(*index);
    return true;
  }

  bool bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    if (!checkBlock(block, state, *param_)) {
      return state.addStackFunction("bootstrap()");
    }

    auto* index = insertBlockHeader(block);
    index->height = height;
    if (!load(state, height == 0)) {
      return state.addStackFunction("bootstrap()");
    }

    auto fullHash = block.getHash();
    if (!block_index_.empty() && !getBlockIndex(fullHash)) {
      return state.Error("block-index-no-genesis");
    }

    repo_->put(*index);
    return true;
  }

 protected:
  virtual void determineBestChain(Chain<block_t>& currentBest,
                                  index_t& indexNew) {
    if (currentBest.tip() == nullptr ||
        currentBest.tip()->chainWork < indexNew.chainWork) {
      currentBest.setTip(&indexNew);
    }
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
