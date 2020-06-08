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
    VBK_LOG_DEBUG("Candidate %s is invalid, skipping FR",
                  indexNew.toPrettyString());
    return;
  }

  bool ret = false;
  auto currentTip = currentBest.tip();
  if (currentTip == nullptr) {
    VBK_LOG_DEBUG("Current tip is nullptr, candidate %s becomes new tip",
                  indexNew.toShortPrettyString());
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
    // other chain won! we already set pop state, so only update tip
    ret = this->setTip(indexNew, state, /* skipSetState=*/true);
    VBK_ASSERT(ret);
  } else {
    VBK_LOG_DEBUG("Active chain won");
    // current chain is better
  }
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
                                  const std::vector<eid_t>& payloads) {
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

  for (const auto& p : reverse_iterate(payloads.begin(), payloads.end())) {
    auto it = std::find(index->payloadIds.begin(), index->payloadIds.end(), p);
    if (it != index->payloadIds.end()) {
      index->payloadIds.erase(it);
    }
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
        fmt::sprintf("Containing block=%s is added on top of invalid chain",
                     index->toPrettyString()));
  }

  bool isOnActiveChain = activeChain_.contains(index);
  if (isOnActiveChain) {
    auto tip = activeChain_.tip();
    VBK_ASSERT(tip != nullptr);
    auto window = (std::max)(0, tip->height - index->height);
    if (window >= param_->getHistoryOverwriteLimit()) {
      return state.Invalid(
          block_t::name() + "-too-late",
          fmt::sprintf(
              "Containing block=%s is too much behind "
              "of active chain tip. Diff %d is more than allowed %d blocks.",
              index->toShortPrettyString(),
              window,
              param_->getHistoryOverwriteLimit()));
    }

    ValidationState dummy;
    bool ret = setTip(*index->pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  for (const auto& p : payloads) {
    auto pid = p.getId();
    if (std::find(index->payloadIds.begin(), index->payloadIds.end(), pid) !=
        index->payloadIds.end()) {
      return state.Invalid(
          block_t::name() + "-duplicate-payloads",
          fmt::sprintf("Containing block=%s already contains payload %s.",
                       index->toPrettyString(),
                       pid.toHex()));
    }
    storage_.payloads().put(p);
    index->payloadIds.push_back(pid);
  }

  // find all affected tips and do a fork resolution
  auto tips = findValidTips<VbkBlock>(*index);
  VBK_LOG_DEBUG(
      "Found %d affected valid tips in %s", tips.size(), block_t::name());
  for (auto* tip : tips) {
    this->determineBestChain(activeChain_, *tip, state);
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
  auto e = VbkEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddVbkEndorsement>(btc(), *this, std::move(e));
  commands.push_back(std::move(cmd));
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  return fmt::sprintf(
      "%s\n%s", VbkTree::toPrettyString(level), cmp_.toPrettyString(level + 2));
}

void VbkBlockTree::removePayloads(const Blob<24>& hash,
                                  const std::vector<eid_t>& payloads) {
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
