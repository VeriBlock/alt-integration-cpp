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

  return current;
}

bool AltTree::bootstrapWithGenesis(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const Payloads& payloads,
                          ValidationState& state,
                          StateChange* stateChange) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid(
        "acceptBlockHeader()", "bad-prev-block", "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  // our current popState is not at desired "index->pprev" state, so change it
  if (popState_ != index->pprev) {
    AltBlockIndex* newPopState = popState_;
    // we need to unapply payloads from this chain until the fork point,
    // and then apply payloads from the fork chain
    if (!unapplyAndApply(
            pop_, &newPopState, *index, state, [this](AltBlockIndex* i) {
              return this->prepo_->get(i->header.getHash());
            })) {
      pop_.rollback();
      return state.addStackFunction("AltTree::acceptBlock");
    }
    popState_ = newPopState;
  }

  // we just add payloads via addPayloads, no need to unapply state
  if (!pop_.addPayloads(payloads, state, stateChange)) {
    return state.addStackFunction("AltTree::acceptBlock");
  }
  // current state has been changed
  popState_ = index;

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const std::vector<Payloads>& payloads,
                          ValidationState& state,
                          StateChange* change) {
  for (const auto& p : payloads) {
    if (!acceptBlock(block, p, state, change)) {
      return false;
    }
  }

  return true;
}

int AltTree::compareThisToOtherChain(AltBlockIndex* other) {
  ValidationState state;

  // create single-use pop manager, which internally combines payloads from
  // both this chain and the other chain
  PopManager combinedPop = pop_;
  AltBlockIndex* combinedPopState = popState_;

  auto height = config_->getBootstrapBlock().height;
  bool ret = apply(
      combinedPop, &combinedPopState, *other, state, [this](AltBlockIndex* i) {
        return prepo_->get(i->header.getHash());
      });

  if (!ret) {
    // other chain has bad payloads (expired?)
    return 1;  // this chain wins
  }

  Chain<AltBlockIndex> thisChain(height, popState_);
  Chain<AltBlockIndex> forkChain(height, other);

  return combinedPop.compareTwoBranches(thisChain, forkChain);
}

void AltTree::unapply(
    PopManager& pop,
    AltBlockIndex** popState,
    AltBlockIndex& to,
    const std::function<std::vector<Payloads>(AltBlockIndex*)>& getPayloads) {
  auto bootstrapHeight = config_->getBootstrapBlock().height;
  Chain<AltBlockIndex> chain(bootstrapHeight, *popState);
  auto* forkPoint = chain.findFork(&to);
  auto* current = chain.tip();
  while (current && current != forkPoint) {
    // unapply payloads
    for (const auto& p : getPayloads(current)) {
      pop.removePayloads(p);
    }
  }
}

bool AltTree::apply(
    PopManager& pop,
    AltBlockIndex** popState,
    AltBlockIndex& to,
    ValidationState& state,
    const std::function<std::vector<Payloads>(AltBlockIndex*)>& getPayloads) {
  Chain<AltBlockIndex> fork(config_->getBootstrapBlock().height, &to);

  auto* current = *popState;
  // move forward from forkPoint to "to" and apply payloads in between

  // exclude fork point itself
  current = fork.next(current);

  while (current) {
    for (const auto& p : getPayloads(current)) {
      if (!pop.addPayloads(p, state)) {
        return false;
      }
      *popState = current;
    }

    if (current != &to) {
      current = fork.next(current);
    } else {
      break;
    }
  }

  assert(*popState == &to);

  // apply our payloads
  pop.commit();

  return true;
}

bool AltTree::unapplyAndApply(
    PopManager& pop,
    AltBlockIndex** popState,
    AltBlockIndex& to,
    ValidationState& state,
    const std::function<std::vector<Payloads>(AltBlockIndex*)>& getPayloads) {
  unapply(pop, popState, to, getPayloads);

  Chain<AltBlockIndex> chain(config_->getBootstrapBlock().height, &to);
  if (chain.contains(&to)) {
    // do not apply payloads as "to" is in current chain and no new payloads
    // will be added
    return true;
  }

  return apply(pop, popState, to, state, getPayloads);
}

void AltTree::invalidateBlockByHash(const AltTree::hash_t& hash) {
  Chain<AltBlockIndex> chain(config_->getBootstrapBlock().height, popState_);
  auto* index = getBlockIndex(hash);
  if (chain.contains(index)) {
    ValidationState state;
    bool ret = unapplyAndApply(
        pop_, &popState_, *index, state, [this](AltBlockIndex* i) {
          return prepo_->get(i->header.getHash());
        });

    // chain contains index, so we should never ever be adding payloads, only
    // removing, therefore we never get false here
    assert(ret);
  }

  // else:
  // we don't care, since currently applied chain does NOT contain
  // block-to-remove, and we do not maintain current best chain
}
