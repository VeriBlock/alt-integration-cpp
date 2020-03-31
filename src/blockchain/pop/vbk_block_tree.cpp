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
    return state.addStackFunction("VbkTree::bootstrapWithChain");
  }

  if (!cmp_.setState(*getBestChain().tip(), state)) {
    return state.addStackFunction("VbkTree::bootstrapWithChain");
  }

  return true;
}

bool VbkBlockTree::bootstrapWithGenesis(ValidationState& state) {
  if (!VbkTree::bootstrapWithGenesis(state)) {
    return state.addStackFunction("VbkTree::bootstrapWithGenesis");
  }

  auto* tip = getBestChain().tip();
  if (!cmp_.setState(*tip, state)) {
    return state.addStackFunction("VbkTree::bootstrapWithGenesis");
  }

  return true;
}

bool VbkBlockTree::acceptBlock(const VbkBlock& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  index_t* index = nullptr;
  // first, validate and create a block
  if (!validateAndAddBlock(block, state, true, &index)) {
    return state.addStackFunction("VbkTree::acceptBlock");
  }

  assert(index != nullptr);

  // second, do stateless validation of payloads
  for (const auto& p : payloads) {
    if (!checkPayloads(p, state, getParams(), btc().getParams())) {
      return state.addStackFunction("VbkTree::acceptBlock");
    }
  }

  // then, do stateful validation of payloads
  if (!payloads.empty() && !cmp_.addAllPayloads(*index, payloads, state)) {
    return state.Invalid("VbkTree::acceptBlock",
                         "vbk-invalid-pop-" + state.GetRejectReason(),
                         state.GetDebugMessage());
  }

  determineBestChain(activeChain_, *index);

  return true;
}

template <>
bool VbkBlockTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<VbkBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        for (const auto& b : index.containingContext.btc) {
          if (!tree().acceptBlock(b, state)) {
            return state.addStackFunction("VbkTree::addPayloads");
          }
        }

        return true;
      },
      [&]() { unapplyContext(index); });
}

template <>
void VbkBlockTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<VbkBlock>& index) {
  // unapply in "forward" order, because result should be same, but doing this
  // way it should be faster due to less number of calls "determineBestChain"
  for (const auto& b : index.containingContext.btc) {
    tree().invalidateBlockByHash(b.getHash());
  }
}

template <>
void addContextToBlockIndex(BlockIndex<VbkBlock>& index,
                            const typename BlockIndex<VbkBlock>::payloads_t& p,
                            const VbkBlockTree::BtcTree& tree) {
  auto& ctx = index.containingContext.btc;

  // only add blocks that are UNIQUE
  std::unordered_set<uint256> set;
  set.reserve(ctx.size());
  for (auto& c : ctx) {
    set.insert(c.getHash());
  }

  auto add = [&](const BtcBlock& b) {
    auto hash = b.getHash();
    // filter context: add only blocks that are unknown and not in current 'ctx'
    if (!tree.getBlockIndex(hash) && !set.count(hash)) {
      ctx.push_back(b);
      set.insert(hash);
    }
  };

  for (const auto& b : p.transaction.blockOfProofContext) {
    add(b);
  }

  add(p.transaction.blockOfProof);
}

template <>
void removeContextFromBlockIndex(BlockIndex<VbkBlock>& index,
                                 const BlockIndex<VbkBlock>::payloads_t& p) {
  auto& ctx = index.containingContext.btc;
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

}  // namespace altintegration
