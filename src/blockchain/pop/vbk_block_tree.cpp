#include <veriblock/blockchain/pop/vbk_block_tree.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      BlockTree::index_t& indexNew) {
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
  auto* forkKeystone = forkIndex->getAncestor(highestKeystoneAtOrBefore(
      forkIndex->height, param_.getKeystoneInterval()));

  int result = 0;
  if (isCrossedKeystoneBoundary(forkKeystone->height,
                                indexNew.height,
                                param_.getKeystoneInterval()) &&
      isCrossedKeystoneBoundary(forkKeystone->height,
                                currentBest.tip()->height,
                                param_.getKeystoneInterval())) {

    // [vbk fork point ... current tip]
    Chain<index_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());

    // [vbk fork point ... new block]
    Chain<index_t> vbkOther(forkKeystone->height, &indexNew);

    result = compare_(vbkCurrentSubchain, vbkOther);
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
}  // namespace altintegration
