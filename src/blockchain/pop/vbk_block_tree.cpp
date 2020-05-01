// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/pop/pop_utils.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      index_t& indexNew,
                                      bool isBootstrap) {
  if (currentBest.tip() == &indexNew) {
    return;
  }

  // do not even consider invalid indices
  if (!indexNew.isValid()) {
    return;
  }

  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return onTipChanged(indexNew, isBootstrap);
  }

  auto ki = param_->getKeystoneInterval();
  const auto* forkKeystone =
      currentBest.findHighestKeystoneAtOrBeforeFork(&indexNew, ki);
  if ((forkKeystone == nullptr) || isBootstrap) {
    // we did not find fork... this can happen only during bootstrap
    return VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  }

  int result = 0;
  auto* bestTip = currentBest.tip();
  if (isCrossedKeystoneBoundary(forkKeystone->height, indexNew.height, ki) &&
      isCrossedKeystoneBoundary(forkKeystone->height, bestTip->height, ki)) {
    // [vbk fork point keystone ... current tip]
    Chain<index_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());

    // [vbk fork point keystone... new block]
    Chain<index_t> vbkOther(forkKeystone->height, &indexNew);

    result = cmp_.comparePopScore(vbkCurrentSubchain, vbkOther);
  }

  if (result < 0) {
    // other chain won!
    //    auto* prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    onTipChanged(indexNew, isBootstrap);
  } else if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  }

  // existing chain is still best
}

bool VbkBlockTree::bootstrapWithChain(int startHeight,
                                      const std::vector<block_t>& chain,
                                      ValidationState& state) {
  if (!VbkTree::bootstrapWithChain(startHeight, chain, state)) {
    return state.Invalid("vbk-bootstrap-chain");
  }

  if (!cmp_.setState(*getBestChain().tip(), state)) {
    return state.Invalid("vbk-set-state");
  }

  return true;
}

bool VbkBlockTree::bootstrapWithGenesis(ValidationState& state) {
  if (!VbkTree::bootstrapWithGenesis(state)) {
    return state.Invalid("vbk-bootstrap-genesis");
  }

  auto* tip = getBestChain().tip();
  if (!cmp_.setState(*tip, state)) {
    return state.Invalid("vbk-set-state");
  }

  return true;
}

// void VbkBlockTree::invalidateBlockByHash(const hash_t& blockHash) {
//  index_t* blockIndex = getBlockIndex(blockHash);
//  if (blockIndex == nullptr) {
//    return;
//  }
//
//  ValidationState state;
//  bool ret = cmp_.setState(*blockIndex->pprev, state);
//  assert(ret);
//
//  BlockTree::invalidateBlockByIndex(*blockIndex);
//
//  ret = cmp_.setState(*activeChain_.tip(), state);
//  assert(ret);
//
//  (void)ret;
//}

bool VbkBlockTree::addPayloads(PopForkComparator& cmp,
                               const VbkBlock& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  auto* index = VbkTree::getBlockIndex(block.getHash());
  if (!index) {
    return state.Invalid("unknown-block",
                         "AddPayloads should be executed on known blocks");
  }

  if (!cmp.addPayloads(*index, payloads, state)) {
    return state.Invalid("bad-payloads-stateful");
  }

  for (auto* tip : findValidTips<VbkBlock>(*index)) {
    determineBestChain(activeChain_, *tip);
  }

  return true;
}

void VbkBlockTree::removePayloads(const VbkBlock& block,
                                  const std::vector<payloads_t>& payloads) {
  auto* index = getBlockIndex(block.getHash());
  if (index != nullptr) {
    removePayloads(index, payloads);
  }
}

void VbkBlockTree::removePayloads(index_t* index,
                                  const std::vector<payloads_t>& payloads) {
  assert(index);
  cmp_.removePayloads(*index, payloads);

  determineBestChain(activeChain_, *index);
}

bool VbkBlockTree::addPayloads(const VbkBlock& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state,
                               bool atomic) {
  if (!atomic) {
    return addPayloads(cmp_, block, payloads, state);
  }

  // execute addPayloads on a temp copy
  PopForkComparator cmp = cmp_;
  bool ret = addPayloads(cmp, block, payloads, state);
  if (ret) {
    // if succeded, update local copy
    cmp_ = cmp;
  }
  return ret;
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  std::string pad(level, ' ');
  ss << VbkTree::toPrettyString(level) << "\n";
  ss << pad << "{comparator=\n" << cmp_.toPrettyString(level + 2);
  ss << "}";
  return ss.str();
}

void VbkBlockTree::removeSubtree(index_t& toRemove) {
  ValidationState state;
  bool ret = false;

  bool isStateSubset =
      cmp_.getIndex()->getAncestor(toRemove.height) == &toRemove;
  if (isStateSubset) {
    ret = cmp_.setState(*toRemove.pprev, state);
    assert(ret);
  }

  bool isOnActiveChain = activeChain_.contains(&toRemove);
  if (isOnActiveChain) {
    activeChain_.setTip(toRemove.pprev);
  }

  base::removeSubtree(
      toRemove, [](index_t& next) { removeAllContainingEndorsements(next); });

  if (isOnActiveChain) {
    // do a fork resolution
    // find new best chain among remaining forks
    for (auto it = base::tips_.begin(); it != base::tips_.end();) {
      index_t* index = *it;
      if (!index->isValid()) {
        it = base::tips_.erase(it);
      } else {
        determineBestChain(activeChain_, *index);
        ++it;
      }
    }
  }

  ret = cmp_.setState(*activeChain_.tip(), state);
  assert(ret);
  (void)ret;
}

void VbkBlockTree::removeSubtree(const hash_t& hash) {
  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    return;
  }

  return removeSubtree(*index);
}

template <>
bool VbkBlockTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<VbkBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        size_t i = 0;
        for (const auto& el : index.containingContext.btc_context) {
          for (const auto& b : el.second) {
            if (!tree().acceptBlock(b, state)) {
              return state.Invalid("vbk-accept-block", i);
            }
            i++;
          }
        }

        return true;
      },
      [&]() { unapplyContext(index); });
}  // namespace altintegration

template <>
void VbkBlockTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<VbkBlock>& index) {
  auto& c = index.containingContext.btc_context;
  std::for_each(
      c.rbegin(),
      c.rend(),
      [&](const std::pair<VbkContext::eid_t,
                          std::vector<std::shared_ptr<BtcBlock>>>& ctx) {
        std::for_each(ctx.second.rbegin(),
                      ctx.second.rend(),
                      [&](const std::shared_ptr<BtcBlock>& b) {
                        tree().removeSubtree(b->getHash());
                      });
      });

}  // namespace altintegration

template <>
void removeContextFromBlockIndex(BlockIndex<VbkBlock>& index,
                                 const BlockIndex<VbkBlock>::payloads_t& p) {
  using eid_t = typename BlockIndex<VbkBlock>::eid_t;

  auto& ctx = index.containingContext.btc_context;
  auto end = ctx.end();

  end = std::remove_if(
      ctx.begin(),
      end,
      [&p](const std::pair<eid_t, std::vector<std::shared_ptr<BtcBlock>>>& el) {
        return p.endorsement.id == el.first;
      });

  ctx.erase(end, ctx.end());
}

template <>
void addContextToBlockIndex(BlockIndex<VbkBlock>& index,
                            const typename BlockIndex<VbkBlock>::payloads_t& p,
                            const BlockTree<BtcBlock, BtcChainParams>& tree) {
  auto& ctx = index.containingContext;

  std::unordered_set<BtcBlock::hash_t> known_blocks;
  for (const auto& e : ctx.btc_context) {
    for (const auto& b : e.second) {
      known_blocks.insert(b->getHash());
    }
  }

  for (const auto& b : p.btc) {
    std::vector<std::shared_ptr<BtcBlock>> btc_vec;
    addBlockIfUnique(b, known_blocks, btc_vec, tree);
    ctx.btc_context.push_back(std::make_pair(p.endorsement.id, btc_vec));
  }
}

}  // namespace altintegration
