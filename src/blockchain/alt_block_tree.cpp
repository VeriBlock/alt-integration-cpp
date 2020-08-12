// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/reversed_range.hpp>
#include <veriblock/storage/batch_adaptor.hpp>

#include "veriblock/algorithm.hpp"
#include "veriblock/command_group_cache.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"

namespace altintegration {

template struct BlockIndex<AltBlock>;

bool AltTree::bootstrap(ValidationState& state) {
  if (base::isBootstrapped()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(std::make_shared<AltBlock>(std::move(block)));
  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  auto height = index->getHeight();

  index->setFlag(BLOCK_APPLIED);
  index->setFlag(BLOCK_BOOTSTRAP);
  base::activeChain_ = Chain<index_t>(height, index);

  VBK_ASSERT(base::isBootstrapped());

  VBK_ASSERT(getBlockIndex(index->getHash()) != nullptr &&
             "getBlockIndex must be able to find the block added by "
             "insertBlockHeader");

  tryAddTip(index);

  return true;
}

// check for the duplicates current index
template <typename Pop>
bool payloadsCheckDuplicates(BlockIndex<AltBlock>& index,
                             ValidationState& state) {
  std::set<typename Pop::id_t> knownIds;

  for (const auto& pid : index.getPayloadIds<Pop>()) {
    if (!knownIds.insert(pid).second) {
      return state.Invalid(
          "ALT-duplicate-payloads-" + Pop::name(),
          fmt::sprintf("Containing block=%s already contains payload %s=%s.",
                       index.toPrettyString(),
                       Pop::name(),
                       pid.toHex()));
    }
  }
  return true;
}

// check that provided payloads exist in the index paylaods
template <typename Pop>
bool payloadsCheckDuplicates(
    BlockIndex<AltBlock>& index,
    const std::vector<typename Pop::id_t>& payloads,
    AltTree&,
    ValidationState& state,
    const std::function<bool(const typename Pop::id_t&)>& remove_function =
        [](const typename Pop::id_t&) { return false; }) {
  auto& v = index.template getPayloadIds<Pop>();
  std::set<typename Pop::id_t> existingPids(v.begin(), v.end());
  for (size_t i = 0; i < payloads.size(); ++i) {
    auto& pid = payloads[i];
    if (!existingPids.insert(pid).second) {
      // remove duplicate id
      if (remove_function(pid)) {
        continue;
      }

      return state.Invalid(
          "ALT-duplicate-payloads-" + Pop::name(),
          fmt::sprintf("Containing block=%s already contains payload %s=%s.",
                       index.toPrettyString(),
                       Pop::name(),
                       pid.toHex()));
    }
  }

  return true;
}

bool vbkBlockCheckDuplicatesInChain(
    BlockIndex<AltBlock>& chainHead,
    const std::vector<typename VbkBlock::id_t>& vbkblocks,
    AltTree& tree,
    ValidationState& state,
    const std::function<bool(const typename VbkBlock::id_t&)>& remove_function =
        [](const typename VbkBlock::id_t&) { return false; }) {
  Chain<BlockIndex<AltBlock>> chain(tree.getParams().getBootstrapBlock().height,
                                    &chainHead);
  for (size_t i = 0; i < vbkblocks.size(); ++i) {
    auto pid = vbkblocks[i];

    // check that VbkBlocks do not exist in the current chain
    auto alt_block_hashes =
        tree.getStorage().getContainingAltBlocks(pid.asVector());
    for (const auto& hash : alt_block_hashes) {
      auto* b_index = tree.getBlockIndex(hash);
      if (chain.contains(b_index)) {
        if (remove_function(pid)) {
          break;
        }

        return state.Invalid(
            "ALT-duplicate-payloads-" + VbkBlock::name(),
            fmt::sprintf("Chain already contains payload %s=%s.",
                         VbkBlock::name(),
                         pid.toHex()));
      }
    }
  }

  return true;
}

template <>
bool payloadsCheckDuplicates<VbkBlock>(
    BlockIndex<AltBlock>& index,
    const std::vector<typename VbkBlock::id_t>& vbkblocks,
    AltTree& tree,
    ValidationState& state,
    const std::function<bool(const typename VbkBlock::id_t&)>&
        remove_function) {
  return vbkBlockCheckDuplicatesInChain(
      index, vbkblocks, tree, state, remove_function);
}

template <typename Pop>
bool payloadsCheckDuplicates(BlockIndex<AltBlock>& index,
                             std::vector<Pop>& payloads,
                             AltTree& tree,
                             ValidationState& state,
                             bool continueOnInvalid = false) {
  auto remove_payload =
      [&payloads, &continueOnInvalid](const typename Pop::id_t& pid) -> bool {
    if (continueOnInvalid) {
      for (auto it = payloads.begin(); it != payloads.end(); ++it) {
        if (it->getId() == pid) {
          payloads.erase(it);
          break;
        }
      }
      return true;
    }
    return false;
  };

  return payloadsCheckDuplicates<Pop>(
      index, map_get_id(payloads), tree, state, remove_payload);
}

template <typename Pop>
void commitPayloadsIds(BlockIndex<AltBlock>& index,
                       const std::vector<Pop>& pop,
                       PayloadsStorage& storage) {
  auto pids = map_get_id(pop);
  index.template insertPayloadIds<Pop>(pids);

  auto containing = index.getHash();
  for (const auto& pid : pids) {
    storage.addAltPayloadIndex(containing, pid.asVector());
  }
}

// check for Payload duplicates in ancestor and descendant blocks
template <typename P>
bool checkNoPayloadDuplicatesInOtherBlocks(
    AltTree& tree,
    AltTree::index_t& index,
    const std::vector<typename P::id_t>& payloads_ids,
    ValidationState& state) {
  auto& storage = tree.getStorage();

  for (size_t i = 0; i < payloads_ids.size(); ++i) {
    auto pid = payloads_ids[i];

    for (const auto& containingBlock :
         storage.getContainingAltBlocks(pid.asVector())) {
      auto* containingIndex = tree.getBlockIndex(containingBlock);
      if (containingIndex == nullptr) {
        return state.Invalid(
            "ALT-duplicate-payloads-" + P::name() + "-ancestor",
            fmt::sprintf(
                "Ancestor block does not exist in the chain %s that we "
                "attempted to add to block=%s.",
                P::name(),
                index.toPrettyString()));
      }

      // is `containing` block of this payload is descendant of `index`?
      if (containingIndex->getHeight() > index.getHeight()) {
        if (containingIndex->getAncestor(index.getHeight()) == &index) {
          // we called AddPayloads(index, pid), where `index` is on the same
          // chain as `containing` and `containing` already has payload with id
          // `pid`. Block that is later, becomes invalid, all its descendants
          // become invalid.
          storage.setValidity(containingBlock, pid, false);
          tree.invalidateSubtree(*containingIndex, BLOCK_FAILED_BLOCK, false);
        } else {
          // `containing` already has `pid`, but is on different chain than
          // `index`
        }
      } else {
        // check if this is an ancestor that contains the `pid`
        if (index.getAncestor(containingIndex->getHeight()) ==
            containingIndex) {
          return state.Invalid(
              "ALT-duplicate-payloads-" + P::name() + "-ancestor",
              fmt::sprintf("Ancestor block=%s already contains %s that we "
                           "attempted to add to block=%s.",
                           containingIndex->toPrettyString(),
                           P::name(),
                           index.toPrettyString()));
        }
      }  // end if
    }    // end for

  }  // end for

  return true;
}

// check for Payload duplicates in ancestor and descendant blocks
template <typename P>
bool checkNoPayloadDuplicatesInOtherBlocks(AltTree& tree,
                                           AltTree::index_t& index,
                                           std::vector<P>& payloads,
                                           ValidationState& state,
                                           bool continueOnInvalid) {
  auto& storage = tree.getStorage();

  for (auto it = payloads.begin(); it != payloads.end();) {
    auto& payload = *it;
    auto pid = payload.getId();
    bool failedPayload = false;

    for (const auto& containingBlock :
         storage.getContainingAltBlocks(pid.asVector())) {
      auto* containingIndex = tree.getBlockIndex(containingBlock);
      VBK_ASSERT(
          containingIndex != nullptr &&
          "state corruption: the storage index and block tree are out of sync");

      // is `containing` block of this payload is descendant of `index`?
      if (containingIndex->getHeight() > index.getHeight()) {
        if (containingIndex->getAncestor(index.getHeight()) == &index) {
          // we called AddPayloads(index, pid), where `index` is on the same
          // chain as `containing` and `containing` already has payload with id
          // `pid`. Block that is later, becomes invalid, all its descendants
          // become invalid.
          storage.setValidity(containingBlock, pid, false);
          tree.invalidateSubtree(
              *containingIndex, BLOCK_FAILED_POP, /*do fr=*/false);
        } else {
          // `containing` already has `pid`, but is on different chain than
          // `index`
        }
      } else {
        // check if this is an ancestor that contains the `pid`
        if (index.getAncestor(containingIndex->getHeight()) ==
            containingIndex) {
          if (continueOnInvalid) {
            failedPayload = true;
            break;
          }

          return state.Invalid(
              "ALT-duplicate-payloads-" + P::name() + "-ancestor",
              fmt::sprintf("Ancestor block=%s already contains %s=%s that we "
                           "attempted to add to block=%s.",
                           containingIndex->toPrettyString(),
                           P::name(),
                           payload.toPrettyString(),
                           index.toPrettyString()));
        }
      }  // end if
    }    // end for

    it = failedPayload ? payloads.erase(it) : ++it;
  }  // end for

  return true;
}

bool AltTree::addPayloads(index_t& index,
                          PopData& payloads,
                          ValidationState& state,
                          bool continueOnInvalid) {
  VBK_LOG_INFO("%s add %d VBK, %d VTB, %d ATV payloads to block %s",
               block_t::name(),
               payloads.context.size(),
               payloads.vtbs.size(),
               payloads.atvs.size(),
               index.toShortPrettyString());

  if (!index.pprev) {
    return state.Invalid(block_t::name() + "-bad-containing-prev",
                         "It is forbidden to add payloads to bootstrap block");
  }

  if (!index.isValid()) {
    return state.Invalid(block_t::name() + "-bad-chain",
                         "Containing block has been marked as invalid");
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool success = setState(*index.pprev, dummy);
    VBK_ASSERT(success);
  }

  // filter payloads
  if (!payloadsCheckDuplicates<VbkBlock>(
          index, payloads.context, *this, state, continueOnInvalid) ||
      !payloadsCheckDuplicates<VTB>(
          index, payloads.vtbs, *this, state, continueOnInvalid) ||
      !payloadsCheckDuplicates<ATV>(
          index, payloads.atvs, *this, state, continueOnInvalid)) {
    return false;
  }

  // check for VTB duplicates in ancestor and descendant blocks
  if (!checkNoPayloadDuplicatesInOtherBlocks(
          *this, index, payloads.vtbs, state, continueOnInvalid) ||
      !checkNoPayloadDuplicatesInOtherBlocks(
          *this, index, payloads.atvs, state, continueOnInvalid)) {
    return false;
  }

  // all payloads checked, they do not contain duplicates. to ensure atomic
  // change, we commit payloads after all 3 vectors have been validated
  commitPayloadsIds<VbkBlock>(index, payloads.context, storage_);
  commitPayloadsIds<VTB>(index, payloads.vtbs, storage_);
  commitPayloadsIds<ATV>(index, payloads.atvs, storage_);

  // save payloads on disk
  storage_.savePayloads(payloads);

  return true;
}

bool AltTree::acceptBlockHeader(const AltBlock& block, ValidationState& state) {
  if (getBlockIndex(block.getHash()) != nullptr) {
    // duplicate
    return true;
  }

  // we must know previous block, but not if `block` is bootstrap block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid(
        block_t::name() + "-bad-prev-block",
        "can not find previous block: " + HexStr(block.previousBlock));
  }

  auto* index = insertBlockHeader(std::make_shared<AltBlock>(block));

  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  if (!index->isValid()) {
    return state.Invalid(block_t::name() + "-bad-chain",
                         "One of previous blocks is invalid. Status=(" +
                             std::to_string(index->status) + ")");
  }

  tryAddTip(index);

  return true;
}

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& tip) {
  VBK_ASSERT(isBootstrapped() && "not bootstrapped");

  auto* index = getBlockIndex(tip);
  VBK_ASSERT_MSG(index, "can not find block %s", HexStr(tip));
  VBK_ASSERT_MSG(index == activeChain_.tip(),
                 "AltTree is at unexpected state: Tip=%s ExpectedTip=%s",
                 activeChain_.tip()->toPrettyString(),
                 index->toPrettyString());

  auto* endorsedBlock = index->getAncestorBlocksBehind(
      alt_config_->getEndorsementSettlementInterval());
  if (endorsedBlock == nullptr) {
    // not enough blocks for payout
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(vbk(), *endorsedBlock);
  auto ret = rewards_.calculatePayouts(vbk(), *endorsedBlock, popDifficulty);
  VBK_LOG_DEBUG("Pop Difficulty=%s for block %s, paying to %d addresses",
                popDifficulty.toPrettyString(),
                index->toShortPrettyString(),
                ret.size());
  return ret;
}

void AltTree::payloadsToCommands(const ATV& atv,
                                 const AltBlock& containing,
                                 std::vector<CommandPtr>& commands) {
  addBlock(vbk(), atv.blockOfProof, commands);

  std::vector<uint8_t> endorsed_hash =
      alt_config_->getHash(atv.transaction.publicationData.header);

  auto e = AltEndorsement::fromContainerPtr(
      atv, containing.getHash(), endorsed_hash);

  auto cmd = std::make_shared<AddAltEndorsement>(vbk(), *this, std::move(e));
  commands.push_back(std::move(cmd));
}

void AltTree::payloadsToCommands(const VTB& vtb,
                                 std::vector<CommandPtr>& commands) {
  auto cmd = std::make_shared<AddVTB>(*this, vtb);
  commands.push_back(std::move(cmd));
}

void AltTree::payloadsToCommands(const VbkBlock& block,
                                 std::vector<CommandPtr>& commands) {
  addBlock(vbk(), block, commands);
}

std::string AltTree::toPrettyString(size_t level) const {
  std::string pad(level, ' ');
  return fmt::sprintf("%sAltTree{blocks=%llu\n%s\n%s\n%s}",
                      pad,
                      base::blocks_.size(),
                      base::toPrettyString(level + 2),
                      cmp_.toPrettyString(level + 2),
                      pad);
}

void AltTree::determineBestChain(index_t& candidate, ValidationState&) {
  auto bestTip = getBestChain().tip();
  VBK_ASSERT(bestTip && "must be bootstrapped");

  if (bestTip == &candidate) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!candidate.isValid()) {
    VBK_LOG_DEBUG("Candidate %s is invalid, skipping FR",
                  candidate.toPrettyString());
    return;
  }

  // if tip==nullptr, then update tip.
  // else - do nothing. AltTree does not (yet) do fork resolution
}

int AltTree::comparePopScore(const AltBlock::hash_t& A,
                             const AltBlock::hash_t& B) {
  auto* left = getBlockIndex(A);
  auto* right = getBlockIndex(B);
  VBK_ASSERT_MSG(left, "unknown 'A' block %s", HexStr(A));
  VBK_ASSERT_MSG(right, "unknown 'B' block %s", HexStr(B));
  VBK_ASSERT(activeChain_.tip() && "not bootstrapped");
  VBK_ASSERT_MSG(activeChain_.tip() == left,
                 "left fork must be applied. Tip: %s, Left: %s",
                 activeChain_.tip()->toPrettyString(),
                 left->toPrettyString());

  ValidationState state;
  // compare current active chain to other chain
  int result = cmp_.comparePopScore(*this, *right, state);
  if (result < 0) {
    // other chain is better, and we already changed 'cmp' state to winner, so
    // just update active chain tip
    activeChain_.setTip(right);
  }

  return result;
}

template <typename Pop, typename Tree, typename Index>
static void clearSideEffects(Tree& tree,
                             Index& index,
                             PayloadsStorage& storage) {
  auto containingHash = index.getHash();
  auto& payloadIds = index.template getPayloadIds<Pop>();
  for (const auto& pid : payloadIds) {
    if (!storage.getValidity(containingHash, pid)) {
      tree.revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
      storage.setValidity(containingHash, pid, true);  // cleanup validity
    }

    storage.removeAltPayloadIndex(containingHash, pid.asVector());
  }
}

void AltTree::removeAllPayloads(index_t& index) {
  VBK_LOG_INFO("%s remove VBK=%d VTB=%d ATV=%d payloads from %s",
               block_t::name(),
               index.getPayloadIds<VbkBlock>().size(),
               index.getPayloadIds<VTB>().size(),
               index.getPayloadIds<ATV>().size(),
               index.toShortPrettyString());

  // we do not allow adding payloads to the genesis block
  VBK_ASSERT(index.pprev && "can not remove payloads from the genesis block");
  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_APPLIED),
                 "block %s is applied",
                 index.toPrettyString());

  if (!index.hasPayloads()) {
    // early return
    return;
  }

  clearSideEffects<VbkBlock>(*this, index, storage_);
  clearSideEffects<VTB>(*this, index, storage_);
  clearSideEffects<ATV>(*this, index, storage_);
  index.clearPayloads();
}

template <typename Payloads, typename BlockIndex>
void removePayloadsIfInvalid(std::vector<Payloads>& p,
                             PayloadsStorage& storage,
                             BlockIndex& index) {
  auto containing = index.getHash();
  auto it = std::remove_if(p.begin(), p.end(), [&](const Payloads& payloads) {
    auto pid = payloads.getId().asVector();
    auto isValid = storage.getValidity(containing, pid);
    // reset validity for this block, as this payload is in temporary block
    if (!isValid) {
      storage.setValidity(containing, pid, true);
    }

    return !isValid;
  });
  p.erase(it, p.end());
}

void AltTree::filterInvalidPayloads(PopData& pop) {
  // return early
  if (pop.empty()) {
    return;
  }

  VBK_LOG_INFO("Called with VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // first, create tmp alt block
  AltBlock tmp;
  ValidationState state;
  {
    auto& tip = *getBestChain().tip();
    tmp.hash = std::vector<uint8_t>(32, 2);
    tmp.previousBlock = tip.getHash();
    tmp.timestamp = tip.getBlockTime() + 1;
    tmp.height = tip.getHeight() + 1;
    bool ret = acceptBlockHeader(tmp, state);
    VBK_ASSERT(ret);
  }

  auto* tmpindex = getBlockIndex(tmp.getHash());
  VBK_ASSERT(tmpindex != nullptr);

  // add all payloads in `continueOnInvalid` mode
  bool success = addPayloads(*tmpindex, pop, state, true);
  VBK_ASSERT(success);

  // setState in 'continueOnInvalid' mode
  setTipContinueOnInvalid(*tmpindex);

  removePayloadsIfInvalid(pop.atvs, storage_, *tmpindex);
  removePayloadsIfInvalid(pop.vtbs, storage_, *tmpindex);
  removePayloadsIfInvalid(pop.context, storage_, *tmpindex);

  VBK_LOG_INFO("After filter VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // at this point `pop` contains only valid payloads

  this->removeSubtree(*tmpindex);
}

bool AltTree::addPayloads(const hash_t& block,
                          const PopData& popData,
                          ValidationState& state) {
  auto copy = popData;
  auto* index = getBlockIndex(block);
  VBK_ASSERT_MSG(index, "can't find block %s", HexStr(block));
  return addPayloads(*index, copy, state, false);
}

bool AltTree::setState(index_t& to, ValidationState& state) {
  bool success = cmp_.setState(*this, to, state);
  if (success) {
    overrideTip(to);
  } else {
    // if setState failed, then 'to' must be invalid
    VBK_ASSERT(!to.isValid());
  }
  return success;
}

void AltTree::overrideTip(index_t& to) {
  VBK_LOG_DEBUG("ALT=\"%s\", VBK=\"%s\", BTC=\"%s\"",
                to.toShortPrettyString(),
                (vbk().getBestChain().tip()
                     ? vbk().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"),
                (btc().getBestChain().tip()
                     ? btc().getBestChain().tip()->toShortPrettyString()
                     : "<empty>"));
  activeChain_.setTip(&to);
  tryAddTip(&to);
}

void AltTree::setTipContinueOnInvalid(AltTree::index_t& to) {
  ValidationState dummy;
  bool success = cmp_.setState(*this, to, dummy, /*continueOnInvalid=*/true);
  VBK_ASSERT(success);
  overrideTip(to);
}

bool AltTree::loadBlock(const AltTree::index_t& index, ValidationState& state) {
  if (!base::loadBlock(index, state)) {
    return false;  // already set
  }

  // load endorsements
  auto containingHash = index.getHash();
  auto* current = getBlockIndex(containingHash);
  VBK_ASSERT(current);

  if (!payloadsCheckDuplicates<VbkBlock>(*current, state) ||
      !payloadsCheckDuplicates<ATV>(*current, state) ||
      !payloadsCheckDuplicates<VTB>(*current, state)) {
    return false;
  }

  if (current->pprev &&
      !vbkBlockCheckDuplicatesInChain(
          *current->pprev, current->getPayloadIds<VbkBlock>(), *this, state)) {
    return false;
  }

  // recover `endorsedBy` and `blockOfProofEndorsements`
  auto window = std::max(
      0, index.getHeight() - getParams().getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsements(*this, chain, *current, state)) {
    return state.Invalid("bad-endorsements");
  }

  if (!checkNoPayloadDuplicatesInOtherBlocks<ATV>(
          *this, *current, current->getPayloadIds<ATV>(), state) ||
      !checkNoPayloadDuplicatesInOtherBlocks<VTB>(
          *this, *current, current->getPayloadIds<VTB>(), state)) {
    return false;
  }

  storage_.addBlockToIndex(*current);

  return true;
}

AltTree::AltTree(const AltTree::alt_config_t& alt_config,
                 const AltTree::vbk_config_t& vbk_config,
                 const AltTree::btc_config_t& btc_config,
                 PayloadsStorage& storagePayloads)
    : alt_config_(&alt_config),
      vbk_config_(&vbk_config),
      btc_config_(&btc_config),
      cmp_(std::make_shared<VbkBlockTree>(
               vbk_config, btc_config, storagePayloads),
           vbk_config,
           alt_config,
           storagePayloads),
      rewards_(alt_config),
      storage_(storagePayloads) {}

void AltTree::removeSubtree(AltTree::index_t& toRemove) {
  storage_.removePayloadsIndex(toRemove);
  base::removeSubtree(toRemove);
}

bool AltTree::loadTip(const AltTree::hash_t& hash, ValidationState& state) {
  if (!base::loadTip(hash, state)) {
    return false;
  }

  auto* tip = activeChain_.tip();
  VBK_ASSERT(tip);
  while (tip) {
    tip->setFlag(BLOCK_APPLIED);
    tip = tip->pprev;
  }

  return true;
}

void AltTree::removeAllPayloads(const AltTree::hash_t& hash) {
  auto* index = getBlockIndex(hash);
  VBK_ASSERT_MSG(index, "does not contain block %s", HexStr(hash));
  return removeAllPayloads(*index);
}

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands(
    const typename AltTree::index_t& index, AltTree& tree) {
  std::vector<CommandGroup> out{};
  std::vector<CommandGroup> payloads_out;
  payloads_out =
      loadCommandsStorage<AltTree, VbkBlock>(DB_VBK_PREFIX, index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  payloads_out = loadCommandsStorage<AltTree, VTB>(DB_VTB_PREFIX, index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  payloads_out = loadCommandsStorage<AltTree, ATV>(DB_ATV_PREFIX, index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  return out;
}

template <typename Payloads>
void removeId(PayloadsStorage& storage,
              BlockIndex<AltBlock>& index,
              const typename Payloads::id_t& pid) {
  auto& payloads = index.template getPayloadIds<Payloads>();
  auto it = std::find(payloads.rbegin(), payloads.rend(), pid);
  VBK_ASSERT(it != payloads.rend());
  index.removePayloadId<Payloads>(pid);
  storage.removeAltPayloadIndex(index.getHash(), pid.asVector());
}

template <>
void removePayloadsFromIndex(PayloadsStorage& storage,
                             BlockIndex<AltBlock>& index,
                             const CommandGroup& cg) {
  // TODO: can we do better?
  if (cg.payload_type_name == &VTB::name()) {
    removeId<VTB>(storage, index, cg.id);
    return;
  }

  if (cg.payload_type_name == &ATV::name()) {
    removeId<ATV>(storage, index, cg.id);
    return;
  }

  if (cg.payload_type_name == &VbkBlock::name()) {
    removeId<VbkBlock>(storage, index, cg.id);
    return;
  }

  VBK_ASSERT(false && "should not reach here");
}

}  // namespace altintegration
