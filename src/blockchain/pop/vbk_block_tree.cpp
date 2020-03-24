#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      BlockTree::index_t& indexNew) {
  auto ki = param_.getKeystoneInterval();
  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return;
  }

  auto* forkIndex = currentBest.findFork(&indexNew);
  // this should never happen. if it is nullptr, it means that we passed
  // `indexNew` index which has no known prev block, which is possible only
  // after logic error, OR error in 'findFork'
  assert(forkIndex != nullptr);

  // last common keystone of two forks
  auto commonKeystoneHeight = highestKeystoneAtOrBefore(forkIndex->height, ki);
  auto* forkKeystone = forkIndex->getAncestor(commonKeystoneHeight);

  int result = 0;
  auto* bestTip = currentBest.tip();
  if (isCrossedKeystoneBoundary(forkKeystone->height, indexNew.height, ki) &&
      isCrossedKeystoneBoundary(forkKeystone->height, bestTip->height, ki)) {
    // [vbk fork point keystone ... current tip]
    Chain<index_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());

    // [vbk fork point keystone ... new block]
    Chain<index_t> vbkOther(forkKeystone->height, &indexNew);

    result = this->comparePopScore(vbkCurrentSubchain, vbkOther);
  }

  if (result < 0) {
    // other chain won!
    auto prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    return addForkCandidate(prevTip, &indexNew);
  } else if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    return VbkTree::determineBestChain(currentBest, indexNew);
  } else {
    // existing chain is still the best
    addForkCandidate(&indexNew, indexNew.pprev);
    return;
  }
}

template <>
bool addPayloads(VbkBlockTree& tree,
                 const Payloads& payloads,
                 ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        auto& btc = tree.btc();

        /// update btc context
        for (const auto& b : payloads.btccontext) {
          if (!btc.acceptBlock(b, state)) {
            return state.addStackFunction("addPayloads");
          }
        }

        /// ADD ALL VTBs
        for (const auto& vtb : payloads.vtbs) {
          if (!checkVTB(vtb, state, tree.getParams(), btc.getParams())) {
            return state.addStackFunction("addVTB");
          }

          // firstly, add btc context blocks
          for (const auto& block : vtb.transaction.blockOfProofContext) {
            if (!btc.acceptBlock(block, state)) {
              return state.addStackFunction("addVTB");
            }
          }
        }
      },
      [&]() { removePayloads(tree, payloads); });
}

template <>
bool removePayloads(VbkBlockTree& tree, const Payloads& payloads) {
  auto& btc = tree.btc();

  /// first, remove VTBs context
  for (const auto& vtb : payloads.vtbs) {
    for (const auto& b : vtb.transaction.blockOfProofContext) {
      btc.invalidateBlockByHash(b.getHash());
    }
  }

  /// remove btc context
  for (const auto& b : payloads.btccontext) {
    btc.invalidateBlockByHash(b.getHash());
  }
}

}  // namespace altintegration
