// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/command_history.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
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

  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return onTipChanged(nullptr, indexNew, isBootstrap);
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
    auto* prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    onTipChanged(prevTip, indexNew, isBootstrap);
    addForkCandidate(prevTip, &indexNew);
  } else if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  } else {
    // existing chain is still the best
    addForkCandidate(&indexNew, indexNew.pprev);
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

void VbkBlockTree::invalidateBlockByHash(const hash_t& blockHash) {
  index_t* blockIndex = getBlockIndex(blockHash);

  if (blockIndex == nullptr) {
    return;
  }

  ValidationState state;
  bool ret = cmp_.setState(*blockIndex->pprev, state);
  assert(ret);

  BlockTree::invalidateBlockByIndex(blockIndex);

  ret = cmp_.setState(*activeChain_.tip(), state);
  assert(ret);
  (void)ret;
}

bool VbkBlockTree::addPayloads(const VbkBlock::hash_t& block,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state, CommandHistory& history) {
  auto* index = getBlockIndex(block);
  if (index == nullptr) {
    return state.Invalid("unknown-block",
                         "AddPayloads should be executed on known blocks");
  }

  CommandHistory internal;
  auto* prevIndex = cmp_.getIndex();
  return tryValidateWithResources(
      [&]() -> bool {
        if (!cmp_.setState(*index, state)) {
          return state.Error("Set state failed");
        }

        // set initial state machine state = current index
        for (size_t i = 0, size = payloads.size(); i < size; i++) {
          const auto& c = payloads[i];

          // containing block must be correct (current)
          auto containingHash = index->getHash();
          if (c.getContainingBlock().getHash() != containingHash) {
            return state.Invalid("vtb-bad-containing-block", i);
          }

          if (!processPayloads(*this, containingHash, c, state, internal)) {
            return state.Invalid("bad-vtb", i);
          }
        }

        history.addFrom(internal);
        // save clears 'internal'
        internal.save(index->commands);

        // if this index is the part of the some fork_chain set to the tip of
        // that fork for the correct determineBestChain() processing
        std::vector<index_t*> forkTips;
        for (const auto& forkChain : fork_chains_) {
          if (forkChain.second.contains(index) ||
              forkChain.second.tip()->getAncestor(index->height) == index) {
            forkTips.push_back(forkChain.second.tip());
          }
        }

        for (const auto& tip : forkTips) {
          determineBestChain(activeChain_, *tip);
        }

        return true;
      },
      [&]() {
        internal.undoAll();

        bool ret = cmp_.setState(*prevIndex, state);
        assert(ret);
        (void)ret;
      });
}

template <>
bool processPayloads<VbkBlockTree>(VbkBlockTree& tree,
                                   const VbkBlock::hash_t& containingHash,
                                   const VTB& p,
                                   ValidationState& state,
                                   CommandHistory& history) {
  auto* containing = tree.getBlockIndex(containingHash);
  if (containing == nullptr) {
    return state.Invalid(
        "vbk-no-containing-block",
        "Can't find VTB's containing block in VBK: " + containingHash.toHex());
  }

  size_t i = 0;
  // process BTC blocks first
  for (const auto& b : p.transaction.blockOfProofContext) {
    if (!addBlock(tree.btc(), b, state, history)) {
      return state.Invalid("btc-accept-context-block", i);
    }
    i++;
  }
  if (!addBlock(tree.btc(), p.transaction.blockOfProof, state, history)) {
    return state.Invalid("btc-accept-blockofproof-block");
  }

  // add endorsement
  auto e = BtcEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddBtcEndorsement>(
      tree.btc(), tree.getParams(), *containing, std::move(e));
  if (!history.exec(cmd, state)) {
    return state.Invalid("vtb-bad-endorsement");
  }
  return true;
}

}  // namespace altintegration
