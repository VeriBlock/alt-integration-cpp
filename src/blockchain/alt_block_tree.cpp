// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <unordered_set>

#include "veriblock/blockchain/pop/pop_utils.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/stateless_validation.hpp"

namespace altintegration {

AltTree::index_t* AltTree::insertBlockHeader(const AltBlock& block) {
  auto hash = block.getHash();
  index_t* current = base::getBlockIndex(hash);
  if (current != nullptr) {
    // it is a duplicate
    return current;
  }

  current = doInsertBlockHeader(std::make_shared<AltBlock>(block));
  current->raiseValidity(BLOCK_VALID_TREE);
  return current;
}

bool AltTree::bootstrap(ValidationState& state) {
  if (!base::blocks_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!base::blocks_.empty() && (getBlockIndex(block.getHash()) == nullptr)) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  tryAddTip(index);

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

  tryAddTip(index);

  return true;
}

void AltTree::invalidateBlock(const hash_t& blockHash,
                              enum BlockStatus reason) {
  index_t* blockIndex = getBlockIndex(blockHash);

  if (blockIndex == nullptr) {
    // no such block
    return;
  }

  invalidateBlock(*blockIndex, reason);
}

void AltTree::invalidateBlock(index_t& blockIndex, enum BlockStatus reason) {
  ValidationState state;

  bool isInSameChain =
      cmp_.getIndex()->getAncestor(blockIndex.height) == &blockIndex;
  if (isInSameChain) {
    bool ret = cmp_.setState(*blockIndex.pprev, state);
    (void)ret;
    assert(ret);
  }

  base::invalidateBlock(blockIndex, reason, [&](index_t& index) -> bool {
    removeAllContainingEndorsements(index);
    // always visit all subtrees
    return true;
  });
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
  if (index != nullptr) {
    cmp_.removePayloads(*index, payloads);
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
    const AltBlock::hash_t& tip, ValidationState& state) {
  auto* index = getBlockIndex(tip);
  if (index == nullptr) {
    state.Error("Block not found");
    return {};
  }

  auto* endorsedBlock = index->getAncestorBlocksBehind(
      alt_config_->getRewardParams().rewardSettlementInterval());
  if (endorsedBlock == nullptr) {
    state.Error("Not enough blocks to get the endorsed block");
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(vbk(), *endorsedBlock);
  return rewards_.calculatePayouts(vbk(), *endorsedBlock, popDifficulty);
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

  if (!cmp.addPayloads(*index, payloads, state)) {
    return state.Invalid("bad-alt-payloads-stateful");
  }

  return true;
}
void AltTree::removeSubtree(index_t& toRemove) {
  bool ret = false;
  ValidationState state;
  auto isInState = cmp_.getIndex()->getAncestor(toRemove.height) == &toRemove;
  if (isInState) {
    ret = cmp_.setState(*toRemove.pprev, state);
    assert(ret);
  }

  base::removeSubtree(toRemove, [](index_t&) {
    /* do nothing */
  });
}
void AltTree::removeSubtree(const hash_t& hash) {
  auto* index = base::getBlockIndex(hash);
  if (!index) {
    return;
  }
  return removeSubtree(*index);
}

std::string AltTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  std::string pad(level, ' ');
  ss << pad << "AltTree{blocks=" << base::blocks_.size() << "\n";
  ss << base::toPrettyString(level + 2) << "\n";
  ss << cmp_.toPrettyString(level + 2) << "\n";
  return ss.str();
}

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        const auto& ctx = index.containingContext;
        // apply context first
        size_t i = 0;
        for (const auto& b : ctx.vbk) {
          if (!tree().acceptBlock(b, state)) {
            return state.Invalid("alt-accept-block", i);
          }
          i++;
        }

        // apply all VTBs
        i = 0;
        for (const auto& vtb : ctx.vtbs) {
          if (!tree().addPayloads(*vtb.containing, {vtb}, state, false)) {
            return state.Invalid("alt-accept-block", i);
          }
          i++;
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

  const auto& ctx = index.containingContext;

  // process VBK context first
  for (const auto& b : ctx.vbk) {
    tree().removeSubtree(b->getHash());
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
  auto& ctx = index.containingContext;

  std::unordered_set<VbkBlock::hash_t> known_vbk_blocks;
  for (const auto& b : ctx.vbk) {
    known_vbk_blocks.insert(b->getHash());
  }

  std::unordered_set<BtcBlock::hash_t> known_btc_blocks;
  for (const auto& vtb : ctx.vtbs) {
    for (const auto& b : vtb.btc) {
      known_btc_blocks.insert(b->getHash());
    }
  }

  // process Vbk context
  for (const auto& b : p.popData.vbk_context) {
    addBlockIfUnique(b, known_vbk_blocks, ctx.vbk, tree);
  }

  // process VTBs
  for (const auto& vtb : p.popData.vtbs) {
    // process VBK blocks
    for (const auto& b : vtb.context) {
      addBlockIfUnique(b, known_vbk_blocks, ctx.vbk, tree);
    }
    addBlockIfUnique(vtb.getContainingBlock(), known_vbk_blocks, ctx.vbk, tree);
    ctx.vtbs.push_back(PartialVTB::fromVTB(vtb));
  }

  // process ATV
  if (p.containsEndorsements()) {
    for (const auto& b : p.popData.atv.context) {
      addBlockIfUnique(b, known_vbk_blocks, ctx.vbk, tree);
    }
    addBlockIfUnique(
        p.popData.atv.containingBlock, known_vbk_blocks, ctx.vbk, tree);
  }
}

template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p) {
  auto& vbk = index.containingContext.vbk;
  auto vbk_end = vbk.end();
  auto removeBlock = [&](const VbkBlock& b) {
    vbk_end = std::remove_if(
        vbk.begin(), vbk_end, [&b](const std::shared_ptr<VbkBlock>& ptr) {
          return *ptr == b;
        });
  };

  auto& vtbs = index.containingContext.vtbs;
  auto vtbs_end = vtbs.end();
  auto removeVTB = [&](const VTB& vtb) {
    removeBlock(vtb.containingBlock);
    std::for_each(vtb.context.rbegin(), vtb.context.rend(), removeBlock);

    vtbs_end =
        std::remove_if(vtbs.begin(), vtbs_end, [&vtb](const PartialVTB& p_vtb) {
          return p_vtb == PartialVTB::fromVTB(vtb);
        });
  };

  // remove vbk_context
  std::for_each(p.popData.vbk_context.rbegin(),
                p.popData.vbk_context.rend(),
                removeBlock);

  // remove ATV containing block
  removeBlock(p.popData.atv.containingBlock);
  // remove ATV context
  std::for_each(p.popData.atv.context.rbegin(),
                p.popData.atv.context.rend(),
                removeBlock);
  // for every VTB, in reverse order
  std::for_each(p.popData.vtbs.rbegin(), p.popData.vtbs.rend(), removeVTB);

  vbk.erase(vbk_end, vbk.end());
  vtbs.erase(vtbs_end, vtbs.end());
}

}  // namespace altintegration
