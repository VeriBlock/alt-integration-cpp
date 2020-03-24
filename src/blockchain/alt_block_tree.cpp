#include "veriblock/blockchain/alt_block_tree.hpp"

using namespace altintegration;

AltTree::index_t* AltTree::getBlockIndex(
    const std::vector<uint8_t>& hash) const {
  auto it = block_index_.find(hash);
  return it == block_index_.end() ? nullptr : it->second.get();
}

AltTree::index_t* AltTree::touchBlockIndex(const hash_t& blockHash) {
  auto it = block_index_.find(blockHash);
  if (it != block_index_.end()) {
    return it->second.get();
  }

  auto* newIndex = new index_t{};
  it = block_index_.insert({blockHash, std::unique_ptr<index_t>(newIndex)})
           .first;
  return it->second.get();
}

AltTree::index_t* AltTree::insertBlockHeader(const AltBlock& block) {
  auto hash = block.getHash();
  index_t* current = getBlockIndex(hash);
  if (current) {
    // it is a duplicate
    return current;
  }

  current = touchBlockIndex(hash);
  current->header = block;
  current->pprev = getBlockIndex(block.previousBlock);
  if (current->pprev != nullptr) {
    current->height = current->pprev->height + 1;
  }

  return current;
}

bool AltTree::bootstrapWithGenesis(ValidationState& state) {
  if (!btc().bootstrapWithGenesis(state)) {
    return state.Error("btc tree already bootstrapped");
  }

  if (!vbk().bootstrapWithGenesis(state)) {
    return state.Error("vbk tree already bootstrapped");
  }

  if (!block_index_.empty()) {
    return state.Error("alt tree already bootstrapped");
  }

  auto block = config_.getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  return true;
}

bool AltTree::setState(const AltBlock::hash_t& hash, ValidationState& state) {
  auto* to = getBlockIndex(hash);
  if (!to) {
    throw std::logic_error(
        "AltTree::setState is called on a block that is unknown in altTree");
  }

  return setState(pop_, &popState_, *to, state);
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const Payloads* payloads,
                          ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("AltTree::acceptBlock",
                         "bad-prev-block",
                         "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (payloads != nullptr) {
    if (!setState(index->pprev->getHash(), state)) {
      return state.addStackFunction("AltTree::acceptBlock");
    }

    // we just add payloads via addPayloads, no need to unapply state
    if (!pop_.addPayloads(*payloads, state)) {
      pop_.rollback();
      return state.addStackFunction("AltTree::acceptBlock");
    }
    pop_.commit();
  }

  // current state has been changed
  popState_ = index;

  return true;
}


int AltTree::compareTwoBranches(const Chain<index_t>& chain1,
                                const Chain<index_t>& chain2) {
  // chains are not empty
  assert(chain1.first() != nullptr);
  assert(chain2.first() != nullptr);

  ValidationState state;

  if (!setState(mgr, &mgrState, *chain1.tip(), state)) {
    if (!setState(mgr, &mgrState, *chain2.tip(), state)) {
      // both chains contain invalid payloads, this is logic error
      throw std::logic_error(
          "AltTree::compareTwoBranches: both chains contain invalid "
          "payloads: " +
          state.GetDebugMessage());
    }

    // chain A contains invalid payloads, and chain B contains valid payloads,
    // so chain2 wins
    return -1;
  }

  // chain1 contains valid payloads. we need to apply payloads from
  // (forkPoint... chain2.tip]

  auto* forkPoint = chain1.findFork(chain2.tip());
  assert(forkPoint != nullptr && "fork point should exist");

  Chain<index_t> subchain1(forkPoint->height, chain1.tip());
  Chain<index_t> subchain2(forkPoint->height, chain2.tip());

  if (!apply(mgr, &mgrState, *chain2.tip(), state)) {
    // chain2 contains invalid payloads, so chain1 wins
    return 1;
  }

  return compare_(mgr.vbk(), subchain1, subchain2);
}
