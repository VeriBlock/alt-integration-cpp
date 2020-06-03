// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/reversed_range.hpp>

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
    VBK_ASSERT(ret);
    return;
  }

  // edge case: connected block is one of 'next' blocks after our current best
  if (indexNew.getAncestor(currentTip->height) == currentTip) {
    // an attempt to connect a NEXT block
    VBK_LOG_DEBUG("%s Candidate is ahead %d blocks, applying them",
                  block_t::name(),
                  indexNew.height - currentTip->height);
    if (!this->setTip(indexNew, state, false)) {
      VBK_ASSERT(!indexNew.isValid());
    }
    return;
  }

  int result = cmp_.comparePopScore(*this, indexNew, state);
  // pop state is already at "best chain"
  if (result == 0) {
    VBK_LOG_DEBUG("Pop scores are equal");
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(
        currentBest, indexNew, state, /* skipSetState=*/false);
  } else if (result < 0) {
    VBK_LOG_DEBUG("Candidate chain won");
    // other chain won! we already set
    ret = this->setTip(indexNew, state, /* skipSetState=*/true);
    VBK_ASSERT(ret);
  } else {
    VBK_LOG_DEBUG("Active chain won");
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
    VBK_LOG_DEBUG("SetTip=%s", to.toPrettyString());
    activeChain_.setTip(&to);
    tryAddTip(&to);
  } else {
    VBK_ASSERT(!to.isValid());
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
  VBK_LOG_DEBUG(
      "remove %d payloads from %s", payloads.size(), block.toPrettyString());
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

  if (payloads.empty()) {
    return;
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    VBK_ASSERT(index->pprev && "can not remove payloads from genesis block");
    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  // we need only ids, so save some cpu cycles by calculating ids once
  std::vector<uint256> pids = map_vector<payloads_t, uint256>(
      payloads, [](const payloads_t& p) { return p.getId(); });

  auto& c = index->commands;
  // iterate over payloads backwards
  for (const auto& p : make_reversed(pids.begin(), pids.end())) {
    // find every payloads in command group (search backwards, as it is likely
    // to be faster)
    auto it = std::find_if(c.rbegin(), c.rend(), [&p](const CommandGroup& g) {
      return g.id == p;
    });

    if (it == c.rend()) {
      // not found
      continue;
    }

    // if this payloads invalidated subtree, we have to re-validate it again
    if (!it->valid) {
      revalidateSubtree(*index, BLOCK_FAILED_POP, /*do fr=*/false);
    }

    // TODO(warchant): fix inefficient erase (does reallocation for every
    // payloads item)
    auto toRemove = --(it.base());
    c.erase(toRemove);
  }

  // find all affected tips and do a fork resolution
  auto tips = findValidTips<VbkBlock>(*index);
  for (auto* tip : tips) {
    ValidationState state;
    determineBestChain(activeChain_, *tip, state);
  }
}

bool VbkBlockTree::addPayloads(const VbkBlock::hash_t& hash,
                               const std::vector<payloads_t>& payloads,
                               ValidationState& state) {
  VBK_LOG_DEBUG("%s add %d payloads to block %s",
                block_t::name(),
                payloads.size(),
                HexStr(hash));
  if (payloads.empty()) {
    return true;
  }

  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    return state.Invalid(block_t::name() + "-bad-containing",
                         "Can not find VTB containing block: " + hash.toHex());
  }

  if (!index->pprev) {
    return state.Invalid(block_t::name() + "-bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return state.Invalid(
        block_t::name() + "-bad-chain",
        fmt::sprintf("Current block=%s is added on top of invalid chain",
                     index->toPrettyString()));
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  /*for (const auto& p : payloads) {
    storage_.payloads().put(p);
    auto pid = p.getId();
    (void)pid;
    ///TODO: add pid in the block index
  }*/

  auto& c = index->commands;
  for (const auto& p : payloads) {
    c.emplace_back();
    auto& g = c.back();
    g.id = p.getId();
    payloadsToCommands(p, g.commands);
  }

  // find all affected tips and do a fork resolution
  auto tips = findValidTips<VbkBlock>(*index);
  for (auto* tip : tips) {
    determineBestChain(activeChain_, *tip, state);
  }

  return index->isValid();
}

void VbkBlockTree::payloadsToCommands(const VTB& p,
                                      std::vector<CommandPtr>& commands) {
  // process BTC context blocks
  for (const auto& b : p.transaction.blockOfProofContext) {
    addBlock(btc(), b, commands);
  }
  // process block of proof
  addBlock(btc(), p.transaction.blockOfProof, commands);

  // add endorsement
  auto e = BtcEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddBtcEndorsement>(btc(), *this, std::move(e));
  commands.push_back(std::move(cmd));
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  return fmt::sprintf(
      "%s\n%s", VbkTree::toPrettyString(level), cmp_.toPrettyString(level + 2));
}

void VbkBlockTree::removePayloads(const Blob<24>& hash,
                                  const std::vector<payloads_t>& payloads) {
  auto index = base::getBlockIndex(hash);
  if (!index) {
    // silently ignore...
    // there's a case when we legitimately can't throw:
    // in command AddVTB we may add a VTB whose containing block is
    // invalid/unknown. removePayloads will immediately be called right after
    // addPayloads, and if we throw here - will break the state.
    return;
  }

  removePayloads(*index->header, payloads);
}

}  // namespace altintegration
