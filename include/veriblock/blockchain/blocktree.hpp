#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <memory>
#include <set>
#include <map>
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
            typename = typename std::enable_if<N == hash_t::size()>::type>
  index_t* getBlockIndex(const Blob<N>& hash) const {
    auto it = block_index_.find(hash);
    return it == block_index_.end() ? nullptr : it->second.get();
  }

  template <size_t N,
            typename = typename std::enable_if<N <= hash_t::size()>::type>
  index_t* getBlockIndexByPrefix(const Blob<N>& hash) const {
    hash_t fullHash = toFullHash(hash);
    auto findBegin = block_index_.lower_bound(fullHash);
    auto findEnd = block_index_.upper_bound(fullHash);

    if (findBegin == block_index_.end()) {
      return nullptr;
    }

    // upper_bound never reaches the end of the map so we make sure
    // to check the upper_bound element as well
    if (findEnd != block_index_.end()) {
      findEnd++;
    }

    for (auto it = findBegin; it != findEnd; it++) {
      auto shortHash = it->first.template trimLE<N>();
      if (shortHash == hash) return it->second.get();
    }

    return nullptr;
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    return acceptBlock(block, state, true);
  }

  const Chain<Block>& getBestChain() const { return this->activeChain_; }

 private:
  std::map<hash_t, std::unique_ptr<index_t>> block_index_;
  Chain<Block> activeChain_;
  std::shared_ptr<BlockRepository<index_t>> repo_;
  std::shared_ptr<ChainParams> param_;

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& hash) {
    hash_t fullHash = toFullHash(hash);
    auto it = block_index_.find(fullHash);
    if (it != block_index_.end()) {
      return it->second.get();
    }

    auto* newIndex = new index_t{};
    it = block_index_.insert({fullHash, std::unique_ptr<index_t>(newIndex)})
             .first;
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
    current->pprev = getBlockIndexByPrefix(block.previousBlock);

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
      bool validateBlock =
          bootstrapGenesis || processedBlocks++ > numBlocksForBootstrap;
      if (acceptBlock(index->header, state, validateBlock)) {
        return state.addStackFunction("load()");
      }
    }

    return true;
  }

  bool acceptBlock(const block_t& block,
                   ValidationState& state,
                   bool validateBlock) {
    if (!checkBlock(block, state, *param_)) {
      return state.addStackFunction("acceptBlockHeader()");
    }

    // we must know previous block
    auto* prev = getBlockIndexByPrefix(block.previousBlock);
    if (prev == nullptr) {
      return state.Invalid("acceptBlockHeader()",
                           "bad-prev-block",
                           "can not find previous block");
    }

    // check difficulty
    if (validateBlock &&
        block.getDifficulty() != getNextWorkRequired(*prev, block, *param_)) {
      return state.Invalid(
          "acceptBlockHeader()", "bad-diffbits", "incorrect proof of work");
    }

    if (!checkBlockTime(*prev, block, state)) {
      return state.addStackFunction("acceptBlockHeader()");
    }

    // check keystones
    if (validateBlock && !validateKeystones(*prev, block)) {
      return state.Invalid(
          "acceptBlockHeader()", "bad-keystones", "incorrect keystones");
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

    if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
      return state.Error("block-index-no-genesis");
    }

    repo_->put(*index);
    return true;
  }

  template <size_t N>
  hash_t toFullHash(const Blob<N>& hash) const {
    // this is how we pad with zeroes from the left
    hash_t fullHash(hash.reverse());
    return fullHash.reverse();
  }

  bool validateKeystones(const BlockIndex<Block>& prevBlock, const Block& block) const;

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
