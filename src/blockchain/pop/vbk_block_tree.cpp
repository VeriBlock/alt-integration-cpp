// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      index_t& indexNew,
                                      ValidationState& state,
                                      bool isBootstrap) {
  if (currentBest.tip() == &indexNew) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!indexNew.isValid()) {
    return;
  }

  bool ret = false;
  auto currentTip = currentBest.tip();
  if (currentTip == nullptr) {
    ret = setTip(indexNew, state, isBootstrap);
    assert(ret);
    return;
  }

  // edge case: connected block is one of 'next' blocks after our current best
  if (indexNew.getAncestor(currentTip->height) == currentTip) {
    // an attempt to connect a NEXT block
    this->setTip(indexNew, state, false);
    return;
  }

  int result = cmp_.comparePopScore(*this, indexNew, state);
  // pop state is already at "best chain"
  if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(
        currentBest, indexNew, state, /* skipSetState=*/false);
  } else if (result < 0) {
    // other chain won! we already set
    ret = this->setTip(indexNew, state, /* skipSetState=*/true);
    assert(ret);
  } else {
    // current chain is better
  }

  (void)ret;
}

bool VbkBlockTree::setTip(index_t& to,
                          ValidationState& state,
                          bool skipSetState) {
  bool changeTip = true;
  if (!skipSetState) {
    changeTip = cmp_.setState(*this, to, state);
  }

  // edge case: if changeTip is false, then new block arrived on top of current
  // active chain, and this block has invalid commands
  if (changeTip) {
    activeChain_.setTip(&to);
  } else {
    assert(!to.isValid());
  }

  return changeTip;
}

bool VbkBlockTree::bootstrapWithChain(int startHeight,
                                      const std::vector<block_t>& chain,
                                      ValidationState& state) {
  if (!VbkTree::bootstrapWithChain(startHeight, chain, state)) {
    return state.Invalid("vbk-bootstrap-chain");
  }

  return true;
}

bool VbkBlockTree::bootstrapWithGenesis(ValidationState& state) {
  if (!VbkTree::bootstrapWithGenesis(state)) {
    return state.Invalid("vbk-bootstrap-genesis");
  }

  return true;
}

void VbkBlockTree::removePayloads(const block_t& block,
                                  const std::vector<payloads_t>& payloads) {
  auto hash = block.getHash();
  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    throw std::logic_error("removePayloads is called on unknown VBK block: " +
                           hash.toHex());
  }

  if (!index->pprev) {
    // we do not add payloads to genesis block, therefore we do not have to
    // remove them
    return;
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    assert(index->pprev && "can not remove payloads from genesis block");
    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    assert(ret);
    (void)ret;
  }

  // remove all matched command groups
  auto& c = index->commands;
  c.erase(std::remove_if(c.begin(),
                         c.end(),
                         [&payloads](const CommandGroup& g) {
                           for (const auto& p : payloads) {
                             if (g == p.getId()) {
                               return true;
                             }
                           }

                           return false;
                         }),
          c.end());

  if (isOnActiveChain) {
    // find all affected tips and do a fork resolution
    auto tips = findValidTips<VbkBlock>(*index);
    for (auto* tip : tips) {
      ValidationState state;
      determineBestChain(activeChain_, *tip, state);
    }
  }
}

bool VbkBlockTree::addPayloads(const VbkBlock::hash_t& hash,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    return state.Invalid("bad-containing",
                         "Can not find VTB containing block: " + hash.toHex());
  }

  if (!index->pprev) {
    return state.Invalid("bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return state.Invalid("bad-chain",
                         "Current block is added on top of invalid chain");
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    assert(ret);
    (void)ret;
  }

  for (const auto& p : payloads) {
    index->commands.emplace_back();
    auto& g = index->commands.back();
    g.id = p.getId();
    payloadsToCommands(*this, p, g.commands);
  }

  // find all affected tips and do a fork resolution
  auto tips = findValidTips<VbkBlock>(*index);
  for (auto* tip : tips) {
    determineBestChain(activeChain_, *tip, state);
  }

  return index->isValid();
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  std::string pad(level, ' ');
  ss << VbkTree::toPrettyString(level) << "\n";
  ss << cmp_.toPrettyString(level + 2);
  ss << pad << "}";
  return ss.str();
}

bool VbkBlockTree::setState(const VbkBlock::hash_t& block,
                            ValidationState& state) {
  auto* index = getBlockIndex(block);
  if (!index) {
    return false;
  }
  return this->setTip(*index, state);
}

template <>
void payloadsToCommands<VbkBlockTree>(VbkBlockTree& tree,
                                      const VTB& p,
                                      std::vector<CommandPtr>& commands) {
  // process BTC context blocks
  for (const auto& b : p.transaction.blockOfProofContext) {
    addBlock(tree.btc(), b, commands);
  }
  // process block of proof
  addBlock(tree.btc(), p.transaction.blockOfProof, commands);

  // add endorsement
  auto e = BtcEndorsement::fromContainerPtr(p);
  auto cmd =
      std::make_shared<AddBtcEndorsement>(tree.btc(), tree, std::move(e));
  commands.push_back(std::move(cmd));
}
}  // namespace altintegration
