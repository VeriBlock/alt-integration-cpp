#include "veriblock/blockchain/alt_block_tree.hpp"

#include <set>

#include "veriblock/blockchain/pop/pop_utils.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
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
  if (current != nullptr) {
    // it is a duplicate
    return current;
  }

  current = touchBlockIndex(hash);
  current->header = std::make_shared<AltBlock>(block);
  current->pprev = getBlockIndex(block.previousBlock);

  if (current->pprev != nullptr) {
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

  if (!block_index_.empty() && (getBlockIndex(block.getHash()) == nullptr)) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  addToChains(index);

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
  if (getBlockIndex(block.getHash()) != nullptr) {
    // duplicate
    return true;
  }

  // we must know previous block, but not if `block` is bootstrap block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  addToChains(index);

  return true;
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
  for (auto* index : removeIndexes) {
    block_index_.erase(index->getHash());
  }

  block_index_.erase(blockIndex.getHash());
}

bool AltTree::addPayloads(const AltBlock& containingBlock,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state,
                          bool atomic) {
  if (!atomic) {
    return addPayloads(cmp_, containingBlock, payloads, state);
  }

  // do a temp copy of comparator
  auto copy = cmp_;
  bool ret = addPayloads(copy, containingBlock, payloads, state);
  if (ret) {
    // if payloads valid, update local copy
    cmp_ = copy;
  }

  return ret;
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
  if (index == nullptr) {
    return state.Error("Can not switch state to an unknown block");
  }

  return cmp_.setState(*index, state);
}

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& block) {
  auto* index = getBlockIndex(block);
  if (index == nullptr) {
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(*index);
  return rewards_.calculatePayouts(*index, popDifficulty);
}

int AltTree::compareTwoBranches(AltTree::index_t* chain1,
                                AltTree::index_t* chain2) {
  if (chain1 == nullptr) {
    throw std::logic_error(
        "compareTwoBranches: AltTree is not aware of chain1");
  }

  if (chain2 == nullptr) {
    throw std::logic_error(
        "compareTwoBranches: AltTree is not aware of chain2");
  }

  if (chain1->getHash() == chain2->getHash()) {
    // equal chains are equal in terms of POP.
    return 0;
  }

  // determine which chain is better
  auto lowestHeight = alt_config_->getBootstrapBlock().height;
  Chain<index_t> chainA(lowestHeight, chain1);
  Chain<index_t> chainB(lowestHeight, chain2);

  if (chainA.contains(chain2) || chainB.contains(chain1)) {
    // We compare 2 blocks from the same chain.
    // In terms of POP, both chains are equal.
    return 0;
  }

  const auto* forkPoint = chainA.findHighestKeystoneAtOrBeforeFork(
      chainB.tip(), alt_config_->getKeystoneInterval());
  if (forkPoint == nullptr) {
    forkPoint = chainB.findHighestKeystoneAtOrBeforeFork(
        chainA.tip(), alt_config_->getKeystoneInterval());
    if (forkPoint == nullptr) {
      throw std::logic_error(
          "compareTwoBranches: No common block between chain A " +
          HexStr(chain1->getHash()) + " and B " + HexStr(chain2->getHash()));
    }
  }

  assert(forkPoint != nullptr && "fork point should exist");

  Chain<index_t> subchain1(forkPoint->height, chainA.tip());
  Chain<index_t> subchain2(forkPoint->height, chainB.tip());

  return cmp_.comparePopScore(subchain1, subchain2);
}

int AltTree::compareTwoBranches(const hash_t& chain1, const hash_t& chain2) {
  auto* i1 = getBlockIndex(chain1);
  auto* i2 = getBlockIndex(chain2);
  return compareTwoBranches(i1, i2);
}

bool AltTree::addPayloads(AltTree::PopForkComparator& cmp,
                          const AltBlock& containingBlock,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  auto hash = containingBlock.getHash();
  auto* index = getBlockIndex(hash);
  if (index == nullptr) {
    return state.Invalid("no-alt-block",
                         "addPayloads can be executed only on existing "
                         "blocks, can not find block " +
                             HexStr(hash));
  }

  // allocate a new element in the stack
  index->containingContext.emplace_back();

  if (!cmp.addPayloads(*index, payloads, state)) {
    index->containingContext.pop_back();
    return state.Invalid("bad-atv-stateful");
  }

  return true;
}

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        if (index.containingContext.empty()) {
          return true;
        }
        const auto& ctx = index.containingContext.back();
        // apply context first
        for (const auto& b : ctx.vbk) {
          if (!tree().acceptBlock(b, state)) {
            return state.Invalid("alt-accept-block");
          }
        }

        // apply all VTBs
        for (const auto& vtb : ctx.vtbs) {
          if (!tree().addPayloads(*vtb.containing, {vtb}, state, false)) {
            return state.Invalid("alt-accept-block");
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

  const auto& ctx = index.containingContext.back();

  // process VBK context first
  for (const auto& b : ctx.vbk) {
    tree().invalidateBlockByHash(b->getHash());
  }

  // step 2, process VTBs
  for (const auto& vtb : ctx.vtbs) {
    auto* containingIndex = tree().getBlockIndex(vtb.containing->getHash());
    if (containingIndex == nullptr) {
      continue;
    }

    tree().removePayloads(containingIndex, {vtb});
  }
}

template <>
void addContextToBlockIndex(BlockIndex<AltBlock>& index,
                            const typename BlockIndex<AltBlock>::payloads_t& p,
                            const VbkBlockTree& tree) {
  assert(!index.containingContext.empty());

  auto& ctx = index.containingContext.back();

  std::unordered_set<VbkBlock::hash_t> known_blocks;
  for (const auto& b : ctx.vbk) {
    known_blocks.insert(b->getHash());
  }

  auto addBlock = [&](const VbkBlock& b) {
    auto hash = b.getHash();

    // filter context: add only blocks that are unknown and not in current 'ctx'
    // if we inserted into known_blocks and tree does not know about this block
    if (known_blocks.insert(hash).second &&
        tree.getBlockIndex(hash) == nullptr) {
      ctx.vbk.push_back(std::make_shared<VbkBlock>(b));
    }
  };

  // process VTBs
  for (const auto& vtb : p.vtbs) {
    for (const auto& b : vtb.context) {
      addBlock(b);
    }
    addBlock(vtb.getContainingBlock());

    ctx.vtbs.push_back(PartialVTB::fromVTB(vtb));
  }

  // process ATV
  if (p.hasAtv) {
    for (const auto& b : p.atv.context) {
      addBlock(b);
    }
    addBlock(p.atv.containingBlock);
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
    vbk_end = std::remove_if(
        vbk.begin(), vbk_end, [&b](const std::shared_ptr<VbkBlock>& ptr) {
          return *ptr == b;
        });
  };

  auto& vtbs = index.containingContext.back().vtbs;
  auto vtbs_end = vtbs.end();
  auto removeVTB = [&](const VTB& vtb) {
    removeBlock(vtb.containingBlock);
    std::for_each(vtb.context.rbegin(), vtb.context.rend(), removeBlock);

    vtbs_end =
        std::remove_if(vtbs.begin(), vtbs_end, [&vtb](const PartialVTB& p_vtb) {
          return p_vtb == PartialVTB::fromVTB(vtb);
        });
  };

  // remove ATV containing block
  removeBlock(p.atv.containingBlock);
  // remove ATV context
  std::for_each(p.atv.context.rbegin(), p.atv.context.rend(), removeBlock);
  // for every VTB, in reverse order
  std::for_each(p.vtbs.rbegin(), p.vtbs.rend(), removeVTB);

  vbk.erase(vbk_end, vbk.end());
  vtbs.erase(vtbs_end, vtbs.end());
}

}  // namespace altintegration
