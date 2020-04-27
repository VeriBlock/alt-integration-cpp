// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/alt_block_tree.hpp"

#include <unordered_set>

#include "veriblock/blockchain/commands/addblock.hpp"
#include "veriblock/blockchain/commands/addendorsement.hpp"
#include "veriblock/blockchain/pop/pop_utils.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/stateless_validation.hpp"

namespace altintegration {

AltTree::index_t* AltTree::getBlockIndex(
    const std::vector<uint8_t>& hash) const {
  auto it = block_index_.find(hash);
  return it == block_index_.end() ? nullptr : it->second.get();
}

AltTree::index_t* AltTree::touchBlockIndex(const hash_t& blockHash) {
  auto it = block_index_.find(blockHash);
  if (it != block_index_.end()) {
    return it->second.get();
  }

  auto newIndex = std::make_shared<index_t>();
  it = block_index_.insert({blockHash, std::move(newIndex)}).first;
  return it->second.get();
}

AltTree::index_t* AltTree::insertBlockHeader(const AltBlock& block) {
  auto hash = block.getHash();
  index_t* current = getBlockIndex(hash);
  if (current != nullptr) {
    // it is a duplicate
    return current;
  }

  current = touchBlockIndex(hash);
  current->header = std::make_shared<AltBlock>(block);
  current->pprev = getBlockIndex(block.previousBlock);

  if (current->pprev != nullptr) {
    // prev block found
    current->height = current->pprev->height + 1;
    current->chainWork = current->pprev->chainWork;
  } else {
    current->height = 0;
    current->chainWork = 0;
  }

  return current;
}

bool AltTree::bootstrap(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && (getBlockIndex(block.getHash()) == nullptr)) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  addToChains(index);

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
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  addToChains(index);

  return true;
}

void AltTree::addToChains(index_t* index) {
  assert(index);

  for (auto& chainTip : chainTips_) {
    if (chainTip == index->pprev) {
      chainTip = index;
      return;
    }
  }

  chainTips_.push_back(index);
}

void AltTree::invalidateBlockByHash(const hash_t& blockHash) {
  index_t* blockIndex = getBlockIndex(blockHash);

  if (blockIndex == nullptr) {
    // no such block
    return;
  }

  invalidateBlockByIndex(*blockIndex);
}

void AltTree::invalidateBlockByIndex(index_t& blockIndex) {
  ValidationState state;
  bool ret = cmp_.setState(*blockIndex.pprev, state);
  (void)ret;
  assert(ret);

  removeAllContainingEndorsements(blockIndex);

  std::set<index_t*> removeIndexes;

  // clear endorsements
  bool once_changed = false;
  for (auto tip = chainTips_.begin(); tip != chainTips_.end();) {
    if ((*tip)->getAncestor(blockIndex.height) == &blockIndex) {
      for (index_t* workBlock = *tip; workBlock != &blockIndex;
           workBlock = workBlock->pprev) {
        removeIndexes.insert(workBlock);
        removeAllContainingEndorsements(*workBlock);
      }
      if (!once_changed) {
        (*tip) = blockIndex.pprev;
        once_changed = true;
      } else {
        tip = chainTips_.erase(tip);
        continue;
      }
    }
    ++tip;
  }

  // clear indexes
  for (auto* index : removeIndexes) {
    block_index_.erase(index->getHash());
  }

  block_index_.erase(blockIndex.getHash());
}

bool AltTree::setState(const AltBlock::hash_t& to, ValidationState& state) {
  auto* index = getBlockIndex(to);
  if (index == nullptr) {
    return state.Error("Can not switch state to an unknown block");
  }

  return cmp_.setState(*index, state);
}

std::map<std::vector<uint8_t>, int64_t> AltTree::getPopPayout(
    const AltBlock::hash_t& block) {
  auto* index = getBlockIndex(block);
  if (index == nullptr) {
    return {};
  }

  auto popDifficulty = rewards_.calculateDifficulty(*index);
  return rewards_.calculatePayouts(*index, popDifficulty);
}

int AltTree::compareTwoBranches(AltTree::index_t* chain1,
                                AltTree::index_t* chain2) {
  if (chain1 == nullptr) {
    throw std::logic_error(
        "compareTwoBranches: AltTree is not aware of chain1");
  }

  if (chain2 == nullptr) {
    throw std::logic_error(
        "compareTwoBranches: AltTree is not aware of chain2");
  }

  if (chain1->getHash() == chain2->getHash()) {
    // equal chains are equal in terms of POP.
    return 0;
  }

  // determine which chain is better
  auto lowestHeight = alt_config_->getBootstrapBlock().height;
  Chain<index_t> chainA(lowestHeight, chain1);
  Chain<index_t> chainB(lowestHeight, chain2);

  if (chainA.contains(chain2) || chainB.contains(chain1)) {
    // We compare 2 blocks from the same chain.
    // In terms of POP, both chains are equal.
    return 0;
  }

  const auto* forkPoint = chainA.findHighestKeystoneAtOrBeforeFork(
      chainB.tip(), alt_config_->getKeystoneInterval());
  if (forkPoint == nullptr) {
    forkPoint = chainB.findHighestKeystoneAtOrBeforeFork(
        chainA.tip(), alt_config_->getKeystoneInterval());
    if (forkPoint == nullptr) {
      throw std::logic_error(
          "compareTwoBranches: No common block between chain A " +
          HexStr(chain1->getHash()) + " and B " + HexStr(chain2->getHash()));
    }
  }

  assert(forkPoint != nullptr && "fork point should exist");

  Chain<index_t> subchain1(forkPoint->height, chainA.tip());
  Chain<index_t> subchain2(forkPoint->height, chainB.tip());

  return cmp_.comparePopScore(subchain1, subchain2);
}

int AltTree::compareTwoBranches(const hash_t& chain1, const hash_t& chain2) {
  auto* i1 = getBlockIndex(chain1);
  auto* i2 = getBlockIndex(chain2);
  return compareTwoBranches(i1, i2);
}

bool AltTree::addPayloads(const AltBlock& containingBlock,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  auto hash = containingBlock.getHash();
  auto* index = getBlockIndex(hash);
  if (index == nullptr) {
    return state.Invalid("no-alt-block",
                         "addPayloads can be executed only on existing "
                         "blocks, can not find block " +
                             HexStr(hash));
  }

  CommandHistory history;
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
            return state.Invalid("altpayloads-bad-containing-block", i);
          }

          if (!processPayloads(*this, containingHash, c, state, history)) {
            return state.Invalid("bad-altpayloads", i);
          }
        }

        history.save(index->commands);

        return true;
      },
      [&]() {
        history.undoAll();

        // rollback to last valid state
        bool ret = cmp_.setState(*prevIndex, state);
        assert(ret);
        (void)ret;
      });
}

std::string AltTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  auto pad = std::string(level, ' ');
  ss << pad << "AltTree{blocks=" << block_index_.size() << "\n";
  ss << pad << "  Comparator state: " << cmp_.getIndex()->toPrettyString();
  for (const auto& b : block_index_) {
    ss << '\n' << pad << "  AltBlock=" << b.second->toPrettyString();
    CommandHistory history(b.second->commands);
    if (history.hasUndo()) {
      ss << '\n' << history.toPrettyString(level + 2);
    }
  }
  ss << pad << "}\n";
  return ss.str();
}

template <>
bool processPayloads<AltTree>(AltTree& tree,
                              const AltBlock::hash_t& containingHash,
                              const AltTree::payloads_t& p,
                              ValidationState& state,
                              CommandHistory& history) {
  auto* containing = tree.getBlockIndex(containingHash);
  if (containing == nullptr) {
    return state.Invalid(
        "alt-no-containing-block",
        "Can't find containing block in ALT tree: " + HexStr(containingHash));
  }

  // start with VTBs
  size_t i = 0;
  for (const auto& vtb : p.vtbs) {
    // process VBK blocks
    for (const auto& b : vtb.context) {
      if (!addBlock(tree.vbk(), b, state, history)) {
        return state.Invalid("vbk-bad-context-block", i);
      }
    }
    if (!addBlock(tree.vbk(), vtb.containingBlock, state, history)) {
      return state.Invalid("vbk-bad-containing-block", i);
    }

    // process VTB content
    if (!tree.vbk().addPayloads(
            vtb.containingBlock.getHash(), {vtb}, state, history)) {
      return state.Invalid("vbk-bad-vtb", i);
    }
    i++;
  }

  // start with context
  i = 0;
  for (const auto& b : p.atv.context) {
    if (!addBlock(tree.vbk(), b, state, history)) {
      return state.Invalid("bad-atvcontext-vbk-block", i);
    }
    i++;
  }

  // process ATV
  if (!p.hasAtv) {
    return true;
  }

  // add block of proof
  if (!addBlock(tree.vbk(), p.atv.containingBlock, state, history)) {
    return state.Invalid("bad-atv-containing-block");
  }

  // add endorsement
  auto e = VbkEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddVbkEndorsement>(
      tree.vbk(), tree.getParams(), *containing, e);
  if (!history.exec(cmd, state)) {
    return state.Invalid("bad-atv-endorsement");
  }

  return true;
}

}  // namespace altintegration
