#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      index_t& indexNew,
                                      bool isBootstrap) {
  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return onTipChanged(indexNew, isBootstrap);
  }

  auto ki = param_->getKeystoneInterval();
  auto* forkKeystone =
      currentBest.findHighestKeystoneAtOrBeforeFork(&indexNew, ki);
  if (!forkKeystone || isBootstrap) {
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
    auto prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    onTipChanged(indexNew, isBootstrap);
    return addForkCandidate(prevTip, &indexNew);
  } else if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    return VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  } else {
    // existing chain is still the best
    addForkCandidate(&indexNew, indexNew.pprev);
    return;
  }
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

bool VbkBlockTree::addPayloads(const VbkBlock& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  auto* index = getBlockIndex(block.getHash());
  if (!index) {
    return state.Invalid("unknown-block",
                         "AddPayloads should be executed on known blocks");
  }

  for (size_t i = 0, size = payloads.size(); i < size; i++) {
    auto& p = payloads[i];

    if (!checkPayloads(p, state, getParams(), btc().getParams())) {
      return state.addIndex(i).Invalid("bad-payloads-stateless");
    }
  }

  // allocate a new element in the stack
  context_t ctx;
  index->containingContext.push_back(ctx);

  if (!cmp_.addPayloads(*index, payloads, state)) {
    index->containingContext.pop_back();
    return state.Invalid("bad-payloads-stateful");
  }

  determineBestChain(activeChain_, *index);

  return true;
}

void VbkBlockTree::removePayloads(const VbkBlock& block,
                                  const std::vector<payloads_t>& payloads) {
  auto* index = getBlockIndex(block.getHash());
  removePayloads(index, payloads);
}

void VbkBlockTree::removePayloads(index_t* index,
                                  const std::vector<payloads_t>& payloads) {
  assert(index);
  cmp_.removePayloads(*index, payloads);
  if (index->containingContext.back().btc.empty()) {
    index->containingContext.pop_back();
  }

  determineBestChain(activeChain_, *index);
}

template <>
bool VbkBlockTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<VbkBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        if (index.containingContext.empty()) {
          return true;
        }
        for (const auto& b : index.containingContext.back().btc) {
          if (!tree().acceptBlock(b, state)) {
            return state.Invalid("vbk-accept-block");
          }
        }

        return true;
      },
      [&]() { unapplyContext(index); });
}  // namespace altintegration

template <>
void VbkBlockTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<VbkBlock>& index) {
  // unapply in "forward" order, because result should be same, but doing this
  // way it should be faster due to less number of calls "determineBestChain"
  if (index.containingContext.empty()) {
    return;
  }
  for (const auto& b : index.containingContext.back().btc) {
    tree().invalidateBlockByHash(b.getHash());
  }

}  // namespace altintegration

template <>
void removeContextFromBlockIndex(BlockIndex<VbkBlock>& index,
                                 const BlockIndex<VbkBlock>::payloads_t& p) {
  if (index.containingContext.empty()) {
    return;
  }

  auto& ctx = index.containingContext.back().btc;
  auto end = ctx.end();
  auto remove = [&](const BtcBlock& b) {
    end = std::remove(ctx.begin(), end, b);
  };

  // update block of proof context
  for (const auto& b : p.transaction.blockOfProofContext) {
    remove(b);
  }

  // and finally add block of proof
  remove(p.transaction.blockOfProof);

  ctx.erase(end, ctx.end());
}

template <>
void addContextToBlockIndex(BlockIndex<VbkBlock>& index,
                            const typename BlockIndex<VbkBlock>::payloads_t& p,
                            const BlockTree<BtcBlock, BtcChainParams>& tree) {
  assert(!index.containingContext.empty());

  auto& ctx = index.containingContext.back().btc;

  // only add blocks that are UNIQUE
  std::unordered_set<uint256> set;
  set.reserve(ctx.size());
  for (const auto& c : ctx) {
    set.insert(c.getHash());
  }

  auto add = [&](const BtcBlock& b) {
    auto hash = b.getHash();
    // filter context: add only blocks that are unknown and not in current 'ctx'
    if (!set.count(hash) && !tree.getBlockIndex(hash)) {
      ctx.push_back(b);
      set.insert(hash);
    }
  };

  for (const auto& b : p.transaction.blockOfProofContext) {
    add(b);
  }

  add(p.transaction.blockOfProof);
}

}  // namespace altintegration
