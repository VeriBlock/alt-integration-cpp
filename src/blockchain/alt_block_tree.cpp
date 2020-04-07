#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/stateless_validation.hpp"

namespace altintegration {

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

  auto newIndex = std::make_shared<index_t>();
  it = block_index_.insert({blockHash, std::move(newIndex)}).first;
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

  if (current->pprev) {
    // prev block found
    current->height = current->pprev->height + 1;
    current->chainWork = current->pprev->chainWork;
  } else {
    current->height = 0;
    current->chainWork = 0;
  }

  return current;
}

void AltTree::addToChains(index_t* index) {
  assert(index);

  for (auto& chainTip : chainTips_) {
    if (chainTip == index->pprev) {
      chainTip = index;
      return;
    }
  }

  chainTips_.push_back(index);
}

bool AltTree::bootstrapWithGenesis(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_.getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  addToChains(index);

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const std::vector<context_t>& context,
                          ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!cmp_.proceedAllEndorsements(*index, context, state)) {
    return state.Invalid("VbkTree::acceptBlock");
  }

  addToChains(index);

  return true;
}

template <>
bool PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    applyContext(const BlockIndex<AltBlock>& index, ValidationState& state) {
  auto apply = [&]() -> bool {
    if (!index.containingContext.empty()) {
      const auto& ctx = index.containingContext.top();

      // step 1
      for (const auto& b : ctx.vbk) {
        if (!tree().acceptBlock(b, {}, state)) {
          return state.Invalid("alt-accept-block");
        }
      }

      // step 2, process VTB info

      for (const auto& vtb_info : ctx.vbkContext) {
        for (const auto& b : vtb_info.contextVbkBlocks) {
          if (!tree().acceptBlock(b, {}, state)) {
            return state.Invalid("alt-accept-block");
          }
        }

        if (!tree().acceptBlock(
                vtb_info.containing, {vtb_info.context}, state)) {
          return state.Invalid("alt-accept-block");
        }
      }
    }

    return true;
  };

  return tryValidateWithResources(apply, [&]() { unapplyContext(index); });
}

template <>
void PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    unapplyContext(const BlockIndex<AltBlock>& index) {
  using vbk_state_machine_t = typename VbkBlockTree::PopForkComparator::sm_t;
  // unapply in "forward" order, because result should be same, but doing this
  // way it should be faster due to less number of calls "determineBestChain"
  if (!index.containingContext.empty()) {
    const auto& ctx = index.containingContext.top();

    // step 1
    for (const auto& b : ctx.vbk) {
      tree().invalidateBlockByHash(b.getHash());
    }

    // step 2, process VTB info
    for (const auto& vtb_info : ctx.vbkContext) {
      auto* temp = tree().getBlockIndex(vtb_info.containing.getHash());
      removeEndorsements(*temp, vtb_info.context.endorsement);
      // update state
      ValidationState state;
      vbk_state_machine_t vbkStateMachine(
          tree().getComparator().getProtectingBlockTree(),
          tree().getComparator().getIndex(),
          tree().getParams());

      vbkStateMachine.unapplyContext(*temp);
      temp->containingContext.pop();
      bool res = vbkStateMachine.applyContext(*temp, state);

      assert(res && "always should be true here");

      if (temp->containingContext.empty()) {
        tree().invalidateBlockByHash(vtb_info.containing.getHash());
        for (const auto& b : vtb_info.contextVbkBlocks) {
          tree().invalidateBlockByHash(b.getHash());
        }
      }
    }
  }
}

template <>
void addContextToBlockIndex(
    BlockIndex<AltBlock>& index,
    const typename BlockIndex<AltBlock>::context_t& context,
    const VbkBlockTree& tree) {
  if (!index.containingContext.empty()) {
    auto& ctx = index.containingContext.top();

    // step 1
    for (const auto& b : context.vbk) {
      if (!tree.getBlockIndex(b.getHash())) {
        ctx.vbk.push_back(b);
      }
    }
    // step 2, process VTB info
    for (const auto& vtb_info : context.vbkContext) {
      auto* temp = tree.getBlockIndex(vtb_info.containing.getHash());

      if (!temp ||
          temp->containingEndorsements.find(vtb_info.context.endorsement.id) ==
              temp->containingEndorsements.end()) {
        ctx.vbkContext.push_back(
            {vtb_info.containing, vtb_info.context, {/* empty vector */}});

        for (const auto& b : vtb_info.contextVbkBlocks) {
          if (!tree.getBlockIndex(b.getHash())) {
            ctx.vbkContext.rbegin()->contextVbkBlocks.push_back(b);
          }
        }
      }
    }
  }
}

}  // namespace altintegration
