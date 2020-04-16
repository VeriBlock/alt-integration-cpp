#include <set>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/pop/pop_utils.hpp"
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
  current->header = std::make_shared<AltBlock>(block);
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

  addToChains(index);

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
  if (getBlockIndex(block.getHash())) {
    // duplicate
    return true;
  }

  // we must know previous block, but not if `block` is bootstrap block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr && block_index_.size() > 1) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  addToChains(index);

  return true;
}

void AltTree::addToChains(index_t* index) {
  assert(index);

  for (size_t i = 0; i < chainTips_.size(); ++i) {
    if (chainTips_[i] == index->pprev) {
      chainTips_[i] = index;
      return;
    }
  }

  chainTips_.push_back(index);
}

void AltTree::invalidateBlockByHash(const hash_t& blockHash) {
  index_t* blockIndex = getBlockIndex(blockHash);

  if (blockIndex == nullptr) {
    // no such block
    return;
  }

  invalidateBlockByIndex(*blockIndex);
}

void AltTree::invalidateBlockByIndex(index_t& blockIndex) {
  ValidationState state;
  bool ret = cmp_.setState(*blockIndex.pprev, state);
  (void)ret;
  assert(ret);

  removeAllContainingEndorsements(blockIndex);

  std::set<index_t*> removeIndexes;

  // clear endorsements
  bool once_changed = false;
  for (auto tip = chainTips_.begin(); tip != chainTips_.end();) {
    if ((*tip)->getAncestor(blockIndex.height) == &blockIndex) {
      for (index_t* workBlock = *tip; workBlock != &blockIndex;
           workBlock = workBlock->pprev) {
        removeIndexes.insert(workBlock);
        removeAllContainingEndorsements(*workBlock);
      }
      if (!once_changed) {
        (*tip) = blockIndex.pprev;
        once_changed = true;
      } else {
        tip = chainTips_.erase(tip);
        continue;
      }
    }
    ++tip;
  }

  // clear indexes
  for (auto index : removeIndexes) {
    block_index_.erase(index->getHash());
  }

  block_index_.erase(blockIndex.getHash());
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
    if (p.hasAtv && !checkATV(p.atv, state, *alt_config_, vbk().getParams())) {
      return state.addIndex(i).Invalid("bad-atv-stateless");
    }
  }

  // allocate a new element in the stack
  context_t ctx;
  index->containingContext.push_back(ctx);

  if (!cmp_.addPayloads(*index, payloads, state)) {
    index->containingContext.pop_back();
    return state.Invalid("bad-atv-stateful");
  }

  return true;
}

void AltTree::removePayloads(const AltBlock& containingBlock,
                             const std::vector<payloads_t>& payloads) {
  auto* index = getBlockIndex(containingBlock.getHash());
  assert(index);

  cmp_.removePayloads(*index, payloads);

  if (index->containingContext.back().empty()) {
    index->containingContext.pop_back();
  }
}

bool AltTree::setState(const AltBlock::hash_t& to, ValidationState& state) {
  auto* index = getBlockIndex(to);
  if (!index) {
    return state.Error("Can not switch state to an unknown block");
  }

  return cmp_.setState(*index, state);
}

int AltTree::compareTwoBranches(AltTree::index_t* chain1,
                                AltTree::index_t* chain2) {
  if (!chain1 && !chain2) {
    throw std::logic_error(
        "compareTwoBranches is called on two nullptr chains");
  }

  if (!chain1) {
    // chain2 is better
    return -1;
  }

  if (!chain2) {
    // chain1 is better
    return 1;
  }

  // determine which chain is better
  auto lowestHeight = alt_config_->getBootstrapBlock().height;
  Chain<index_t> chainA(lowestHeight, chain1);
  Chain<index_t> chainB(lowestHeight, chain2);

  auto* forkPoint = chainA.findHighestKeystoneAtOrBeforeFork(
      chainB.tip(), alt_config_->getKeystoneInterval());
  assert(forkPoint != nullptr && "fork point should exist");

  Chain<index_t> subchain1(forkPoint->height, chainA.tip());
  Chain<index_t> subchain2(forkPoint->height, chainB.tip());

  return cmp_.comparePopScore(subchain1, subchain2);
}

int AltTree::compareTwoBranches(const hash_t& chain1, const hash_t& chain2) {
  auto i1 = getBlockIndex(chain1);
  auto i2 = getBlockIndex(chain2);
  return compareTwoBranches(i1, i2);
}

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        if (index.containingContext.empty()) {
          return true;
        }
        auto& ctx = index.containingContext.back();
        // step 1
        for (const auto& b : ctx.vbk) {
          if (!tree().acceptBlock(b, state)) {
            return state.Invalid("alt-accept-block");
          }
        }

        // step 2, process VTBs
        for (const auto& vtb : ctx.vtbs) {
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
    return index && index->containingContext.empty();
  };

  auto& ctx = index.containingContext.back();

  // step 1
  for (const auto& b : ctx.vbk) {
    if (check(b)) {
      tree().invalidateBlockByHash(b.getHash());
    }
  }

  // step 2, process VTBs
  for (const auto& vtb : ctx.vtbs) {
    auto* containingIndex = tree().getBlockIndex(vtb.containingBlock.getHash());
    if (!containingIndex) continue;

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

  auto& ctx = index.containingContext.back();

  auto addBlock = [&](const VbkBlock& b, std::vector<VbkBlock>& blocks) {
    auto hash = b.getHash();
    // filter context: add only blocks that are unknown and not in current 'ctx'
    if (!tree.getBlockIndex(hash)) {
      blocks.push_back(b);
    }
  };

  // add context from ATV
  if (p.hasAtv) {
    for (const auto& b : p.atv.context) {
      addBlock(b, ctx.vbk);
    }
    addBlock(p.atv.containingBlock, ctx.vbk);
  }

  // step 2, process VTBs
  for (const auto& vtb : p.vtbs) {
    auto* temp = tree.getBlockIndex(vtb.getContainingBlock().getHash());

    if (!temp || temp->containingEndorsements.find(
                     BtcEndorsement::fromContainer(vtb).id) ==
                     temp->containingEndorsements.end()) {
      ctx.vtbs.push_back(vtb);
      ctx.vtbs.rbegin()->context.clear();
      for (const auto& b : vtb.context) {
        addBlock(b, ctx.vtbs.rbegin()->context);
      }
    }
  }
}

template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p) {
  if (index.containingContext.empty()) {
    return;
  }

  auto& vbk = index.containingContext.back().vbk;
  auto vbk_end = vbk.end();
  auto removeBlock = [&](const VbkBlock& b) {
    vbk_end = std::remove(vbk.begin(), vbk_end, b);
  };

  auto& vtbs = index.containingContext.back().vtbs;
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
