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
    return state.addStackFunction("VbkBlockTree::bootstrapWithChain");
  }

  if (!cmp_.setState(*getBestChain().tip(), state)) {
    return state.addStackFunction("VbkBlockTree::bootstrapWithChain");
  }

  return true;
}

bool VbkBlockTree::bootstrapWithGenesis(ValidationState& state) {
  if (!VbkTree::bootstrapWithGenesis(state)) {
    return state.addStackFunction("VbkBlockTree::bootstrapWithGenesis");
  }

  auto* tip = getBestChain().tip();
  if (!cmp_.setState(*tip, state)) {
    return state.addStackFunction("VbkBlockTree::bootstrapWithGenesis");
  }

  return true;
}

bool VbkBlockTree::acceptBlock(const VbkBlock& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state,
                               StateChange* change) {
  index_t* index = nullptr;
  if (!validateAndAddBlock(block, state, true, &index)) {
    return state.addStackFunction("VbkBlockTree::acceptBlock");
  }

  assert(index != nullptr);

  if (!cmp_.addAllPayloads(*index, payloads, state)) {
    invalidateBlockByHash(index->getHash());
    return state.Invalid("VbkBlockTree::acceptBlock",
                         "vbk-invalid-pop",
                         state.GetDebugMessage());
  }

  // save payloads on disk

  if (change) {
    for (const auto& payload : payloads) {
      change->saveVbkPayloads(payload);
    }
  }

  determineBestChain(activeChain_, *index);

  return true;
}

template <>
bool PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::applyContext(const VTB& payloads,
                                                   ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        auto& btc = tree();

        // check VTB
        if (!checkVTB(payloads, state, params(), btc.getParams())) {
          return state.addStackFunction("PopStateMachine::applyContext");
        }

        // and update context
        for (const auto& block : payloads.transaction.blockOfProofContext) {
          if (!btc.acceptBlock(block, state)) {
            return state.addStackFunction("PopStateMachine::applyContext");
          }
        }

        // add block of proof
        if (!btc.acceptBlock(payloads.transaction.blockOfProof, state)) {
          return state.addStackFunction("PopStateMachine::applyContext");
        }

        return true;
      },
      [&]() { unapplyContext(payloads); });
}

template <>
void PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::unapplyContext(const VTB& payloads) {
  auto& btc = tree();

  // remove VTB context
  for (const auto& b : payloads.transaction.blockOfProofContext) {
    btc.invalidateBlockByHash(b.getHash());
  }

  // remove block of proof
  btc.invalidateBlockByHash(payloads.transaction.blockOfProof.getHash());
}

template <>
bool PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::addPayloads(const VTB& p,
                                                  ValidationState& state) {
  // endorsement validity window
  auto window = params().getEndorsementSettlementInterval();
  auto minHeight = index_->height >= window ? index_->height - window : 0;
  Chain<BlockIndex<VbkBlock>> chain(minHeight, index_);

  auto endorsedHeight = p.transaction.publishedBlock.height;
  assert(index_->height > endorsedHeight);
  if (index_->height - endorsedHeight > window) {
    return state.Invalid(
        "PopStateMachine::addPayloads", "expired", "Endorsement expired");
  }

  auto* endorsed = chain[endorsedHeight];
  if (!endorsed) {
    return state.Invalid("PopStateMachine::addPayloads",
                         "no-endorsed-block",
                         "No block found on endorsed block height");
  }

  if (endorsed->getHash() != p.transaction.publishedBlock.getHash()) {
    return state.Invalid("PopStateMachine::addPayloads",
                         "block-differs",
                         "Endorsed VBK block is on a different chain");
  }

  auto endorsement = BtcEndorsement::fromContainer(p);
  auto* blockOfProof = tree_.getBlockIndex(endorsement.blockOfProof);
  if (!blockOfProof) {
    return state.Invalid("PopStateMachine::addPayloads",
                         "block-of-proof-not-found",
                         "Can not find block of proof in BTC");
  }

  if (!tree_.getBestChain().contains(blockOfProof)) {
    return state.Invalid(
        "PopStateMachine::addPayloads",
        "block-of-proof-not-on-main-chain",
        "Block of proof has been reorganized and no loger on a main chain");
  }

  auto* duplicate = chain.findBlockContainingEndorsement(endorsement, window);
  if (duplicate) {
    // found duplicate
    return state.Invalid("PopStateMachine::addPayloads",
                         "duplicate",
                         "Found duplicate endorsement on the same chain");
  }

  index_->containingPayloads.push_back(p.getId());

  auto pair = index_->containingEndorsements.insert(
      {endorsement.id, std::make_shared<BtcEndorsement>(endorsement)});
  assert(pair.second && "there's a duplicate in endorsement map");

  auto* eptr = pair.first->second.get();
  endorsed->endorsedBy.push_back(eptr);

  return true;
}

}  // namespace altintegration
