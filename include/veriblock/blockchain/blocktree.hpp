#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <memory>
#include <set>
#include <unordered_map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

/**
 * BlockTree is a tree of blocks with single "bootstrap" block as root.
 * @tparam Block
 */
template <typename Block>
struct BlockTree {
  using block_t = Block;
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;

  BlockTree(std::shared_ptr<BlockRepository<index_t>> repo)
      : repo_(std::move(repo)) {}

  /**
   * Bootstrap blockchain with a single bootstrap block on given height.
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @param height bootstrap block height
   * @param block bootstrap block header
   * @return true if blockchain has been loaded successfully. False, if
   * blockchain stored in BlockRepository had some invalid data.
   */
  bool bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    if (!checkBlock(block, state)) {
      return state.addStackFunction("bootstrap()");
    }

    auto* index = insertBlockHeader(block);
    index->height = height;
    if (!load(state)) {
      return state.addStackFunction("bootstrap()");
    }

    if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
      return state.Error("block-index-no-genesis");
    }

    return true;
  }

  index_t* getBlockIndex(const hash_t& hash) {
    auto it = block_index_.find(hash);
    return it == block_index_.end() ? nullptr : &it->second;
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    if (!checkBlock(block, state)) {
      return state.addStackFunction("acceptBlockHeader()");
    }

    // make a copy of hash to let it extend to hash_t size
    hash_t previousBlockHash(block.previousBlock);
    auto it = block_index_.find(previousBlockHash);
    if (it == block_index_.end()) {
      return state.Invalid("acceptBlockHeader()",
                           "no-prev-block",
                           "can not find previous block");
    }

    // TODO: maybe some other validations?
    // - difficulty calculator(s)?
    // - block too new?
    // - block too old?

    auto* index = insertBlockHeader(block);
    assert(index != nullptr &&
           "insertBlockHeader should have never returned nullptr");
    repo_->put(*index);
    return true;
  }

  const Chain<Block>& getBestChain() const { return this->activeChain_; }

 private:
  std::unordered_map<hash_t, index_t> block_index_;
  Chain<Block> activeChain_;
  std::shared_ptr<BlockRepository<index_t>> repo_;

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& hash) {
    auto it = block_index_.find(hash);
    if (it != block_index_.end()) {
      return &it->second;
    }

    it = block_index_.insert({hash, index_t{}}).first;
    return &it->second;
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
    // make a copy of hash to let it extend to hash_t size
    hash_t previousBlockHash(block.previousBlock);
    current->pprev = getBlockIndex(previousBlockHash);

    if (current->pprev) {
      // prev block found
      current->height = current->pprev->height + 1;
      current->chainWork = current->pprev->chainWork + block.getBlockProof();
    } else {
      current->height = 0;
      current->chainWork = block.getBlockProof();
    }

    determineBestChain(activeChain_, *current);

    return current;
  }

  bool load(ValidationState& state) {
    auto cursor = repo_->getCursor();

    // load blocks into memory
    for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
      auto value = cursor->value();
      auto hash = value.header.getHash();

      // TODO: add consensus params to checkProofOfWork?
      if (!checkProofOfWork(value.header, state)) {
        return state.Error("load-bad-proof-of-work");
      }

      index_t* index = touchBlockIndex(hash);
      *index = value;
      // make a copy of hash to let it extend to hash_t size
      hash_t previousBlockHash(value.header.previousBlock);
      index->pprev = touchBlockIndex(previousBlockHash);
    }

    // fill block_index map
    std::vector<std::pair<height_t, index_t*>> sortedByHeight;
    sortedByHeight.reserve(block_index_.size());
    for (auto& item : block_index_) {
      sortedByHeight.push_back({item.second.height, &item.second});
    }
    std::sort(sortedByHeight.begin(), sortedByHeight.end());
    for (const auto& item : sortedByHeight) {
      index_t* index = item.second;
      index->chainWork =
          index->pprev ? index->pprev->chainWork + index->header.getBlockProof()
                       : 0;
      determineBestChain(activeChain_, *index);
    }

    return true;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
