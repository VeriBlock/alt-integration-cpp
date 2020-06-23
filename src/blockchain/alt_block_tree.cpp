// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <veriblock/blockchain/blockchain_storage_util.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/reversed_range.hpp>

#include "veriblock/algorithm.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"

namespace altintegration {

bool AltTree::bootstrap(ValidationState& state) {
  if (!base::blocks_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(std::make_shared<AltBlock>(std::move(block)));

  VBK_ASSERT(index != nullptr &&
             "insertBlockHeader should have never returned nullptr");

  if (!base::blocks_.empty() && (getBlockIndex(index->getHash()) == nullptr)) {
    return state.Error("block-index-no-genesis");
  }

  determineBestChain(base::activeChain_, *index, state, true);

  tryAddTip(index);

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
                          const PopData& popData,
                          ValidationState& state) {
  if (!addPayloads(index, popData.context, state)) {
    return false;
  }

  if (!addPayloads(index, popData.vtbs, state)) {
    return false;
  }
  if (!addPayloads(index, popData.atvs, state)) {
    return false;
  }

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
  auto* endorsed_index = getBlockIndex(endorsed_hash);
  VBK_ASSERT(endorsed_index);

  auto e = AltEndorsement::fromContainerPtr(
      atv, containing.getHash(), endorsed_hash, endorsed_index->height);

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

bool AltTree::saveToStorage(PopStorage& storage, ValidationState& state) {
  saveBlocks(storage, vbk().btc());
  saveBlocks(storage, vbk());
  saveBlocks(storage, *this);
  return state.IsValid();
}

bool AltTree::loadFromStorage(const PopStorage& storage,
                              ValidationState& state) {
  bool ret = loadBlocks(storage, vbk().btc(), state);
  if (!ret) return state.IsValid();
  ret = loadBlocks(storage, vbk(), state);
  if (!ret) return state.IsValid();
  return loadBlocks(storage, *this, state);
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

void AltTree::determineBestChain(Chain<index_t>& currentBest,
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

  auto currentTip = currentBest.tip();
  if (currentTip == nullptr) {
    VBK_LOG_DEBUG("Current tip is nullptr, candidate %s becomes new tip",
                  indexNew.toShortPrettyString());
    bool ret = setTip(indexNew, state, isBootstrap);
    VBK_ASSERT(ret);
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

  removePayloads(*index, popData);
}

void AltTree::removePayloads(index_t& index, const PopData& popData) {
  if (!index.pprev) {
    // we do not add payloads to genesis block, therefore we do not have to
    // remove them
    return;
  }

  bool isOnActiveChain = activeChain_.contains(&index);
  if (isOnActiveChain) {
    VBK_ASSERT(index.pprev && "can not remove payloads from genesis block");
    ValidationState dummy;
    bool ret = setTip(*index.pprev, dummy, false);
    VBK_ASSERT(ret);
  }

  removePayloads(index, popData.context, false);
  removePayloads(index, popData.vtbs, false);
  removePayloads(index, popData.atvs, false);
}

bool AltTree::setTip(AltTree::index_t& to,
                     ValidationState& state,
                     bool skipSetState) {
  bool changeTip = true;
  if (!skipSetState) {
    changeTip = cmp_.setState(*this, to, state);
  }

  // edge case: if changeTip is false, then new block arrived on top of
  // current active chain, and this block has invalid commands
  if (changeTip) {
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
  } else {
    VBK_ASSERT(!to.isValid());
  }

  // true if tip has been changed
  return changeTip;
}

template <typename pop_t>
std::vector<CommandGroup> loadCommands_(
    const typename AltTree::index_t& index,
    AltTree& tree,
    const std::shared_ptr<PayloadsRepository<pop_t>>& prep) {
  auto& pids = index.getPayloadIds<pop_t, typename pop_t::id_t>();
  std::vector<CommandGroup> out{};
  for (const auto& pid : pids) {
    pop_t payloads;
    if (!prep->get(pid, &payloads)) {
      throw StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    CommandGroup cg(pid.asVector(), payloads.valid, pop_t::name());
    tree.payloadsToCommands(payloads, cg.commands);
    out.push_back(cg);
  }
  return out;
}

template <>
std::vector<CommandGroup> loadCommands_(
    const typename AltTree::index_t& index,
    AltTree& tree,
    const std::shared_ptr<PayloadsRepository<ATV>>& prep) {
  auto& pids = index.getPayloadIds<ATV, typename ATV::id_t>();
  std::vector<CommandGroup> out{};
  for (const auto& pid : pids) {
    ATV payloads;
    if (!prep->get(pid, &payloads)) {
      throw StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    CommandGroup cg(pid.asVector(), payloads.valid, ATV::name());
    tree.payloadsToCommands(payloads, *index.header, cg.commands);
    out.push_back(cg);
  }
  return out;
}

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands<AltTree>(
    const typename AltTree::index_t& index, AltTree& tree) {
  std::vector<CommandGroup> out{};
  std::vector<CommandGroup> payloads_out;

  payloads_out = loadCommands_<VbkBlock>(
      index, tree, PayloadsBaseStorage<VbkBlock>::prepo_);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());

  payloads_out =
      loadCommands_<VTB>(index, tree, PayloadsBaseStorage<VTB>::prepo_);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());

  payloads_out =
      loadCommands_<ATV>(index, tree, PayloadsBaseStorage<ATV>::prepo_);
  out.insert(out.end(), payloads_out.begin(), payloads_out.end());

  return out;
}

template <>
void PopStorage::saveBlocks(
    const std::unordered_map<typename AltBlock::prev_hash_t,
                             std::shared_ptr<BlockIndex<AltBlock>>>& blocks) {
  auto batch = BlocksStorage<BlockIndex<AltBlock>>::brepo_->newBatch();
  if (batch == nullptr) {
    throw BadIOException("Cannot create BlockRepository write batch");
  }

  for (const auto& block : blocks) {
    auto& index = *(block.second);
    batch->put(index);

    for (const auto& e : index.containingEndorsements) {
      saveEndorsements<typename BlockIndex<AltBlock>::endorsement_t>(*e.second);
    }
  }
  batch->commit();
}

uint8_t getBlockProof(const AltBlock&) { return 0; }

}  // namespace altintegration
