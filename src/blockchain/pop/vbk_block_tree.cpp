#include <veriblock/blockchain/pop/vbk_block_tree.hpp>

namespace AltIntegrationLib {

void VbkBlockTree::determineBestChain(Chain<block_t>& currentBest,
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
      forkIndex->height, param_->getKeystoneInterval()));

  int result = 0;
  if (isCrossedKeystoneBoundary(forkKeystone->height,
                                indexNew.height,
                                param_->getKeystoneInterval()) &&
      isCrossedKeystoneBoundary(forkKeystone->height,
                                currentBest.tip()->height,
                                param_->getKeystoneInterval())) {
    // [vbk fork point ... current tip]
    Chain<block_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());
    auto pkcCurrent =
        getProtoKeystoneContext(vbkCurrentSubchain, btc_, erepo_, *param_);
    auto kcCurrent = getKeystoneContext(pkcCurrent, btc_);

    // [vbk fork point ... new block]
    Chain<block_t> vbkOther(forkKeystone->height, &indexNew);
    auto pkcOther = getProtoKeystoneContext(vbkOther, btc_, erepo_, *param_);
    auto kcOther = getKeystoneContext(pkcOther, btc_);

    result = compare_(kcCurrent, kcOther);
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
}  // namespace AltIntegrationLib
