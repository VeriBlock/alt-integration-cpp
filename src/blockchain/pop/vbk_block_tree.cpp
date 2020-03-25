#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      index_t& indexNew) {
  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return;
  }

  auto ki = param_.getKeystoneInterval();
  auto* forkKeystone =
      currentBest.findHighestKeystoneAtOrBeforeFork(&indexNew, ki);

  // this should never happen. if it is nullptr, it means that we passed
  // `indexNew` index which has no known prev block, which is possible only
  // after logic error, OR error in 'findFork'
  assert(forkKeystone != nullptr);

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

  ValidationState state;
  if (result < 0) {
    // other chain won!
    auto prevTip = currentBest.tip();
    setTip(currentBest, indexNew);
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
bool PopStateMachine<VbkBlockTree::BtcTree,
                           BlockIndex<VbkBlock>,
                           VbkChainParams>::addPayloads(const VTB& vtb,
                                                        ValidationState&
                                                            state) {
  return tryValidateWithResources(
      [&]() -> bool {
        auto& btc = tree();

        // check VTB
        if (!checkVTB(vtb, state, params(), btc.getParams())) {
          return state.addStackFunction("VbkTree::addPayloads");
        }

        // and update context
        for (const auto& block : vtb.transaction.blockOfProofContext) {
          if (!btc.acceptBlock(block, state)) {
            return state.addStackFunction("VbkTree::addPayloads");
          }
        }

        return true;
      },
      [&]() { removePayloads(vtb); });
}

template <>
void PopStateMachine<VbkBlockTree::BtcTree,
                           BlockIndex<VbkBlock>,
                           VbkChainParams>::removePayloads(const VTB& vtb) {
  auto& btc = tree();

  /// remove VTB context
  for (const auto& b : vtb.transaction.blockOfProofContext) {
    btc.invalidateBlockByHash(b.getHash());
  }
}

}  // namespace altintegration
