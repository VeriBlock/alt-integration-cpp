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

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const std::vector<AltPayloads>& payloads,
                          ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  // do stateless validation of payloads
  for (size_t i = 0; i < payloads.size(); ++i) {
    if (!checkPayloads(payloads[i].alt.atv, state, vbk_config_)) {
      return state.addIndex(i).Invalid("alt-check-payloads");
    }

    for (const auto& vtb : payloads[i].vtbs) {
      if (!checkPayloads(vtb, state, vbk_config_, btc_config_)) {
        return state.addIndex(i).Invalid("alt-check-payloads");
      }
    }
  }

  if (!cmp_.proceedAllPayloads(*index, payloads, state)) {
    return state.Invalid("VbkTree::acceptBlock");
  }

  addToChains(index);

  return true;
}

template <>
bool PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    applyContext(const BlockIndex<AltBlock>& /*index*/,
                 ValidationState& /*state*/) {
  return true;
}

template <>
void PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    unapplyContext(const BlockIndex<AltBlock>& /*index*/) {}

template <>
void addContextToBlockIndex(
    BlockIndex<AltBlock>& /*index*/,
    const typename BlockIndex<AltBlock>::payloads_t& /*p*/,
    const VbkBlockTree& /*tree*/) {}

}  // namespace altintegration
