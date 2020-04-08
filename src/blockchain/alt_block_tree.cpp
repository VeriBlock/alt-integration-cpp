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

bool AltTree::bootstrap(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  (void)index;

  return true;
}

bool AltTree::addPayloads(const AltBlock& containingBlock,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  auto hash = containingBlock.getHash();
  auto* index = getBlockIndex(hash);
  if (!index) {
    return state.Invalid("no-alt-block",
                         "addPayloads can be executed only on existing "
                         "blocks, can not find block " +
                             HexStr(hash));
  }

  for (size_t i = 0, size = payloads.size(); i < size; i++) {
    auto& p = payloads[i];
    if (p.hasAtv && !checkPayloads(p.atv, state, vbk().getParams())) {
      return state.addIndex(i).Invalid("bad-atv-stateless");
    }
  }

  // allocate a new element in the stack
  context_t ctx;
  index->containingContext.push(ctx);

  if (!cmp_.addPayloads(*index, payloads, state)) {
    index->containingContext.pop();
    return state.Invalid("bad-atv-stateful");
  }

  return true;
}

void AltTree::removePayloads(const AltBlock& containingBlock,
                             const std::vector<payloads_t>& payloads) {
  auto* index = getBlockIndex(containingBlock.getHash());
  assert(index);

  cmp_.removePayloads(*index, payloads);

  if (index->containingContext.top().vbk.empty() &&
      index->containingContext.top().vtbs.empty()) {
    index->containingContext.pop();
  }
}

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        if (index.containingContext.empty()) {
          return true;
        }

        // step 1
        for (const auto& b : index.containingContext.top().vbk) {
          if (!tree().acceptBlock(b, state)) {
            return state.Invalid("alt-accept-block");
          }
        }

        // step 2, process VTBs
        for (const auto& vtb : index.containingContext.top().vtbs) {
          for (const auto& b : vtb.context) {
            if (!tree().acceptBlock(b, state)) {
              return state.Invalid("alt-accept-block");
            }
          }

          auto* containingIndex =
              tree().getBlockIndex(vtb.containingBlock.getHash());
          if (!containingIndex) {
            if (!tree().acceptBlock(vtb.containingBlock, state)) {
              return state.Invalid("alt-accept-block");
            }
          }
          if (!containingIndex ||
              containingIndex->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtb).id) ==
                  containingIndex->containingEndorsements.end()) {
            if (!tree().addPayloads(vtb.containingBlock, {vtb}, state)) {
              return state.Invalid("alt-accept-block");
            }
          }
        }

        return true;
      },
      [&] { unapplyContext(index); });
}

template <>
void AltTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<AltBlock>& index) {
  // unapply in "forward" order, because result should be same, but doing this
  // way it should be faster due to less number of calls "determineBestChain"

  if (index.containingContext.empty()) {
    return;
  }

  auto check = [&](const VbkBlock& block) -> bool {
    auto* index = tree().getBlockIndex(block.getHash());
    return !index && index->containingContext.empty();
  };

  // step 1
  for (const auto& b : index.containingContext.top().vbk) {
    if (check(b)) {
      tree().invalidateBlockByHash(b.getHash());
    }
  }

  // step 2, process VTBs
  for (const auto& vtb : index.containingContext.top().vtbs) {
    auto* containingIndex = tree().getBlockIndex(vtb.containingBlock.getHash());
    tree().removePayloads(containingIndex, {vtb});

    if (containingIndex->containingContext.empty()) {
      tree().invalidateBlockByHash(containingIndex->getHash());
    }

    for (const auto& b : vtb.context) {
      if (check(b)) {
        tree().invalidateBlockByHash(b.getHash());
      }
    }
  }
}

template <>
void addContextToBlockIndex(BlockIndex<AltBlock>& index,
                            const typename BlockIndex<AltBlock>::payloads_t& p,
                            const VbkBlockTree& tree) {
  assert(!index.containingContext.empty());

  auto& ctx = index.containingContext.top().vbk;

  // only add blocks that are UNIQUE
  std::unordered_set<uint256> set;
  set.reserve(ctx.size());
  for (const auto& c : ctx) {
    set.insert(c.getHash());
  }

  auto addBlock = [&](const VbkBlock& b) {
    auto hash = b.getHash();
    // filter context: add only blocks that are unknown and not in current 'ctx'
    if (!set.count(hash) && !tree.getBlockIndex(hash)) {
      ctx.push_back(b);
      set.insert(hash);
    }
  };

  // add context from ATV
  if (p.hasAtv) {
    for (const auto& b : p.atv.context) {
      addBlock(b);
    }
    addBlock(p.atv.containingBlock);
  }

  // step 2, process VTBs
  for (const auto& vtb : p.vtbs) {
    auto* temp = tree.getBlockIndex(vtb.getContainingBlock().getHash());

    if (!temp || temp->containingEndorsements.find(
                     BtcEndorsement::fromContainer(vtb).id) ==
                     temp->containingEndorsements.end()) {
      index.containingContext.top().vtbs.push_back(vtb);
    }
  }
}

template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p) {
  if (index.containingContext.empty()) {
    return;
  }

  auto& vbk = index.containingContext.top().vbk;
  auto vbk_end = vbk.end();
  auto removeBlock = [&](const VbkBlock& b) {
    vbk_end = std::remove(vbk.begin(), vbk_end, b);
  };

  auto& vtbs = index.containingContext.top().vtbs;
  auto vtbs_end = vtbs.end();
  auto removeVTB = [&](const VTB& vtb) {
    vtbs_end = std::remove(vtbs.begin(), vtbs_end, vtb);
  };

  // remove ATV context
  std::for_each(p.atv.context.rbegin(), p.atv.context.rend(), removeBlock);
  // for every VTB, in reverse order
  std::for_each(p.vtbs.rbegin(), p.vtbs.rend(), removeVTB);

  vbk.erase(vbk_end, vbk.end());
  vtbs.erase(vtbs_end, vtbs.end());
}

}  // namespace altintegration
