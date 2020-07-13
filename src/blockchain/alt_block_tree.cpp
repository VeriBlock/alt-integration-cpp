// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/reversed_range.hpp>
#include <veriblock/storage/batch.hpp>
#include <veriblock/storage/blockchain_storage_util.hpp>
#include "veriblock/algorithm.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/command_group_cache.hpp"

namespace altintegration {

bool AltTree::bootstrap(ValidationState& state) {
  if (base::isBootstrapped()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(std::make_shared<AltBlock>(std::move(block)));
  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  auto height = block.getHeight();

  index->setFlagSetDirty(BLOCK_APPLIED);
  index->setFlagSetDirty(BLOCK_BOOTSTRAP);
  base::activeChain_ = Chain<index_t>(height, index);

  VBK_ASSERT(base::isBootstrapped());

  VBK_ASSERT(getBlockIndex(index->getHash()) != nullptr &&
             "getBlockIndex must be able to find the block added by "
             "insertBlockHeader");

  tryAddTip(index);

  return true;
}

template <typename Index, typename Pop>
bool handleAddPayloads(Index& index,
                       std::vector<Pop>& payloads,
                       ValidationState& state,
                       bool continueOnInvalid = false) {
  auto& payloadIds = index.template getPayloadIds<Pop, typename Pop::id_t>();
  std::set<typename Pop::id_t> existingPids(payloadIds.begin(),
                                            payloadIds.end());

  for (auto it = payloads.begin(); it != payloads.end();) {
    auto pid = it->getId();
    if (!existingPids.insert(pid).second) {
      if (continueOnInvalid) {
        // remove duplicate id
        it = payloads.erase(it);
        continue;
      }

      return state.Invalid(
          "ALT-duplicate-payloads",
          fmt::sprintf("Containing block=%s already contains payload %s=%s.",
                       index.toPrettyString(),
                       Pop::name(),
                       pid.toHex()));
    }

    payloadIds.push_back(pid);
    ++it;
  }

  return true;
}

bool AltTree::addPayloads(const AltBlock::hash_t& containing,
                          const PopData& popData,
                          ValidationState& state) {
  auto* index = getBlockIndex(containing);
  if (!index) {
    return state.Invalid(block_t::name() + "-bad-block",
                         "Can't find containing block");
  }
  return addPayloads(*index, popData, state);
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

  if (!handleAddPayloads(index, payloads.context, state, continueOnInvalid)) {
    return false;
  }

  if (!handleAddPayloads(index, payloads.vtbs, state, continueOnInvalid)) {
    return false;
  }

  if (!handleAddPayloads(index, payloads.atvs, state, continueOnInvalid)) {
    return false;
  }

  storagePayloads_.savePayloads(payloads);

  return true;
}

bool AltTree::validatePayloads(const AltBlock& block,
                               const PopData& popData,
                               ValidationState& state) {
  return validatePayloads(block.getHash(), popData, state);
}

bool AltTree::validatePayloads(const AltBlock::hash_t& block_hash,
                               const PopData& popData,
                               ValidationState& state) {
  auto* index = getBlockIndex(block_hash);

  if (!index) {
    return state.Invalid(block_t::name() + "bad-block",
                         "Can't find containing block");
  }

  if (!addPayloads(*index, popData, state)) {
    VBK_LOG_DEBUG(
        "%s Can not add payloads: %s", block_t::name(), state.toString());
    return state.Invalid(block_t::name() + "-addPayloadsTemporarily");
  }

  if (!setState(*index, state)) {
    VBK_LOG_DEBUG("%s Statefully invalid payloads: %s",
                  block_t::name(),
                  state.toString());

    removePayloads(*index, popData);
    return state.Invalid(block_t::name() + "-addPayloadsTemporarily");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
  if (getBlockIndex(block.getHash()) != nullptr) {
    // duplicate
    return true;
  }

  // we must know previous block, but not if `block` is bootstrap block
  auto* prev = getBlockIndex(block.getPreviousBlock());
  if (prev == nullptr) {
    return state.Invalid(
        block_t::name() + "-bad-prev-block",
        "can not find previous block: " + HexStr(block.getPreviousBlock()));
  }

  auto* index = insertBlockHeader(std::make_shared<AltBlock>(block));

  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  if (!index->isValid()) {
    return state.Invalid(block_t::name() + "-bad-chain",
                         "One of previous blocks is invalid. Status=(" +
                             std::to_string(index->getStatus()) + ")");
  }

  tryAddTip(index);

  return true;
}

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& tip, ValidationState& state) {
  auto* index = getBlockIndex(tip);
  if (index == nullptr) {
    state.Error("Block not found");
    return {};
  }

  auto* endorsedBlock = index->getAncestorBlocksBehind(
      alt_config_->getEndorsementSettlementInterval());
  if (endorsedBlock == nullptr) {
    state.Error("Not enough blocks to get the endorsed block");
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
  addBlock(vbk(), atv.containingBlock, commands);

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

int AltTree::comparePopScore(const AltBlock::hash_t& hleft,
                             const AltBlock::hash_t& hright) {
  auto* left = getBlockIndex(hleft);
  if (!left) {
    throw std::logic_error("AltTree: unknown 'left' block");
  }

  auto* right = getBlockIndex(hright);
  if (!right) {
    throw std::logic_error("AltTree: unknown 'other' block");
  }

  if (activeChain_.tip() != left) {
    throw std::logic_error(
        "AltTree: left fork must be applied. Call SetState(left) before fork "
        "resolution.");
  }

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

void AltTree::removePayloads(const AltBlock::hash_t& hash,
                             const PopData& popData) {
  auto* index = base::getBlockIndex(hash);
  if (!index) {
    throw std::logic_error("removePayloads is called on unknown ALT block: " +
                           HexStr(hash));
  }

  return removePayloads(*index, popData);
}

template <typename Tree, typename Index, typename Pop, typename Storage>
void handleRemovePayloads(Tree& tree,
                          Index& index,
                          const std::vector<Pop>& payloads,
                          Storage& storage) {
  std::vector<typename Pop::id_t> pids = map_vector<Pop, typename Pop::id_t>(
      payloads, [](const Pop& p) { return p.getId(); });

  auto& payloadIds = index.template getPayloadIds<Pop, typename Pop::id_t>();

  for (const auto& pid : pids) {
    auto it = std::find(payloadIds.begin(), payloadIds.end(), pid);
    if (it == payloadIds.end()) {
      continue;
    }

    if (!storage.template isValid<Pop, Index>(pid, index)) {
      tree.revalidateSubtree(index, BLOCK_FAILED_POP, false);
    }

    payloadIds.erase(it);
    // TODO: do we want to erase payloads from repository?
  }
}

void AltTree::removePayloads(index_t& index, const PopData& payloads) {
  VBK_LOG_INFO("%s remove VBK=%d VTB=%d ATV=%d payloads from %s",
               block_t::name(),
               payloads.context.size(),
               payloads.vtbs.size(),
               payloads.atvs.size(),
               index.toShortPrettyString());

  // we do not allow adding payloads to the genesis block
  VBK_ASSERT(index.pprev && "can not remove payloads from the genesis block");

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    ValidationState dummy;
    bool success = setState(*index.pprev, dummy);
    VBK_ASSERT(success);
  }

  handleRemovePayloads(*this, index, payloads.atvs, storagePayloads_);
  handleRemovePayloads(*this, index, payloads.vtbs, storagePayloads_);
  handleRemovePayloads(*this, index, payloads.context, storagePayloads_);
}

template <typename Payloads, typename Storage, typename BlockIndex>
void removePayloadsIfInvalid(std::vector<Payloads>& p, Storage& storage, BlockIndex& index) {
  auto it = std::remove_if(p.begin(), p.end(), [&](const Payloads& payloads) {
    auto isValid =
        storage.template isValid<Payloads, BlockIndex>(payloads.getId(), index);
    return !isValid;
  });
  p.erase(it, p.end());
}

void AltTree::filterInvalidPayloads(PopData& pop) {
  // return early
  if (pop.context.empty() && pop.vtbs.empty() && pop.atvs.empty()) {
    return;
  }

  VBK_LOG_INFO("Called with VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // first, create tmp alt block
  ValidationState state;
  auto& tip = *getBestChain().tip();
  AltBlock tmp(std::vector<uint8_t>(32, 2),
               tip.getHash(),
               tip.getBlockTime() + 1,
               tip.getHeight() + 1);
  bool ret = acceptBlock(tmp, state);
  VBK_ASSERT(ret);

  auto* tmpindex = getBlockIndex(tmp.getHash());
  VBK_ASSERT(tmpindex != nullptr);

  // add all payloads in `continueOnInvalid` mode
  bool success = addPayloads(*tmpindex, pop, state, true);
  VBK_ASSERT(success);

  // setState in 'continueOnInvalid' mode
  success = setTipContinueOnInvalid(*tmpindex, state);
  VBK_ASSERT(success);

  removePayloadsIfInvalid(pop.atvs, storagePayloads_, *tmpindex);
  removePayloadsIfInvalid(pop.vtbs, storagePayloads_, *tmpindex);
  removePayloadsIfInvalid(pop.context, storagePayloads_, *tmpindex);

  VBK_LOG_INFO("After filter VBK=%d VTB=%d ATV=%d",
               pop.context.size(),
               pop.vtbs.size(),
               pop.atvs.size());

  // at this point `pop` contains only valid payloads

  removeSubtree(*tmpindex);
}

bool AltTree::addPayloads(AltTree::index_t& index,
                          const PopData& popData,
                          ValidationState& state) {
  auto copy = popData;
  return addPayloads(index, copy, state, false);
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
  VBK_LOG_INFO("ALT=\"%s\", VBK=\"%s\", BTC=\"%s\"",
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

bool AltTree::setTipContinueOnInvalid(AltTree::index_t& to,
                                      ValidationState& state) {
  bool success = cmp_.setState(*this, to, state, /*continueOnInvalid=*/true);
  VBK_ASSERT(success);

  overrideTip(to);

  // true if tip has been changed
  return success;
}

bool AltTree::loadBlock(const AltTree::index_t& index, ValidationState& state) {
  if (!base::loadBlock(index, state)) {
    return false;  // already set
  }

  // load endorsements
  auto* current = getBlockIndex(index.getHash());
  VBK_ASSERT(current);

  // recover `endorsedBy`
  auto window = std::max(
      0, index.getHeight() - getParams().getEndorsementSettlementInterval());
  Chain<index_t> chain(window, current);
  if (!recoverEndorsedBy(*this, chain, *current, state)) {
    return state.Invalid("load-block");
  }

  return true;
}

bool AltTree::addPayloads(const AltBlock& containing,
                          const PopData& popData,
                          ValidationState& state) {
  return addPayloads(containing.getHash(), popData, state);
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
      storagePayloads_(storagePayloads) {}

template <typename pop_t>
std::vector<CommandGroup> loadCommands_(const typename AltTree::index_t& index,
                                        AltTree& tree,
                                        const PayloadsRepository<pop_t>& prep) {
  auto& pids = index.getPayloadIds<pop_t, typename pop_t::id_t>();
  std::vector<CommandGroup> out{};
  for (const auto& pid : pids) {
    pop_t payloads;
    if (!prep.get(pid, &payloads)) {
      throw db::StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    CommandGroup cg(pid.asVector(), payloads.valid, pop_t::name());
    tree.payloadsToCommands(payloads, cg.commands);
    out.push_back(cg);
  }
  return out;
}

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands(
    const typename AltTree::index_t& index, AltTree& tree) {
  std::vector<CommandGroup> out{};
  std::vector<CommandGroup> payloads_out;
  payloads_out = loadCommandsStorage<AltTree, VbkBlock>(index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  payloads_out = loadCommandsStorage<AltTree, VTB>(index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  payloads_out = loadCommandsStorage<AltTree, ATV>(index, tree);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());
  return out;
}

namespace {
bool removeId(std::vector<uint256>& pop, const uint256& id) {
  auto it = std::find(pop.rbegin(), pop.rend(), id);
  if (it == pop.rend()) {
    return false;
  }

  auto toRemove = --(it.base());
  pop.erase(toRemove);
  return true;
}

bool removeId(std::vector<VbkBlock::id_t>& pop, const uint256& id) {
  auto it = std::find_if(pop.rbegin(), pop.rend(), [&](const uint96& a) {
    return uint256(a) == id;
  });
  if (it == pop.rend()) {
    return false;
  }

  auto toRemove = --(it.base());
  pop.erase(toRemove);
  return true;
}
}  // namespace

template <>
void removePayloadsFromIndex(BlockIndex<AltBlock>& index,
                             const CommandGroup& cg) {
  // TODO: can we do better?

  // TODO: remove CG ID from payload ids
  /*if (cg.payload_type_name == VTB::name()) {
    bool ret = removeId(index.vtbids, cg.id);
    VBK_ASSERT(ret);
    return;
  }

  if (cg.payload_type_name == ATV::name()) {
    bool ret = removeId(index.atvids, cg.id);
    VBK_ASSERT(ret);
    return;
  }

  if (cg.payload_type_name == VbkBlock::name()) {
    bool ret = removeId(index.vbkblockids, cg.id);
    VBK_ASSERT(ret);
    return;
  }*/
  (void)index;
  (void)cg;

  VBK_ASSERT(false && "should not reach here");
}

}  // namespace altintegration
