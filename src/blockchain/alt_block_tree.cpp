#include "veriblock/blockchain/alt_block_tree.hpp"

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
  if (current) {
    // it is a duplicate
    return current;
  }

  current = touchBlockIndex(hash);
  current->header = block;
  current->pprev = getBlockIndex(block.previousBlock);
  if (current->pprev != nullptr) {
    current->height = current->pprev->height + 1;
  }

  return current;
}

bool AltTree::bootstrapWithGenesis(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("alt tree already bootstrapped");
  }

  auto block = config_.getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  return true;
}

bool AltTree::setState(const AltTree::index_t& index, ValidationState& state) {
  return cmp_.setState(const_cast<index_t&>(index), state);
}

bool AltTree::setState(const AltBlock::hash_t& hash, ValidationState& state) {
  auto* to = getBlockIndex(hash);
  if (!to) {
    throw std::logic_error(
        "AltTree::setState is called on a block that is unknown in altTree");
  }

  return setState(*to, state);
}

bool AltTree::acceptBlock(const AltBlock& block,
                          const Payloads* payloads,
                          ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("AltTree::acceptBlock",
                         "bad-prev-block",
                         "can not find previous block");
  }

  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (payloads != nullptr) {
    if (!setState(index->pprev->getHash(), state)) {
      return state.addStackFunction("AltTree::acceptBlock");
    }

    // we just add payloads via addPayloads, no need to unapply state
    if (!addPayloads(*this, *payloads, state)) {
      return state.addStackFunction("AltTree::acceptBlock");
    }
  }

  // current state has been changed
  if (!setState(*index, state)) {
    return state.addStackFunction("AltTree::acceptBlock");
  }

  return true;
}

int AltTree::compareTwoBranches(index_t* chain1, index_t* chain2) {
  if (!chain1 && !chain2) {
    // chains are equal
    return 0;
  }

  if (!chain1) {
    // chain2 is better
    return -1;
  }

  if (!chain2) {
    // chain1 is better
    return 1;
  }

  // determine which chain is better
  auto lowestHeight = config_.getBootstrapBlock().height;
  Chain<index_t> chainA(lowestHeight, chain1);
  Chain<index_t> chainB(lowestHeight, chain2);

  auto* forkPoint = chainA.findHighestKeystoneAtOrBeforeFork(
      chainB.tip(), config_.getKeystoneInterval());
  assert(forkPoint != nullptr && "fork point should exist");

  Chain<index_t> subchain1(forkPoint->height, chainA.tip());
  Chain<index_t> subchain2(forkPoint->height, chainB.tip());

  return cmp_(subchain1, subchain2);
}

template <>
bool addPayloads(AltTree& tree,
                 const Payloads& payloads,
                 ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        auto& vbk = tree.vbk();

        // does checkVTB internally
        if (!addPayloads(vbk, payloads, state)) {
          return state.addStackFunction("AltTree::addPayloads");
        }

        /// update vbk context
        for (const auto& b : payloads.vbkcontext) {
          if (!vbk.acceptBlock(b, state)) {
            return state.addStackFunction("AltTree::addPayloads");
          }
        }

        /// ADD ALL VTBs
        for (const auto& vtb : payloads.vtbs) {
          // no need to checkVTB, because first addPayloads does it

          // add vbk context blocks
          for (const auto& block : vtb.context) {
            if (!vbk.acceptBlock(block, state)) {
              return state.addStackFunction("AltTree::addPayloads");
            }
          }
        }

        /// ADD ATV
        if (payloads.alt.hasAtv) {
          // check if atv is statelessly valid
          if (!checkATV(payloads.alt.atv, state, vbk.getParams())) {
            return state.addStackFunction("AltTree::addPayloads");
          }

          /// apply atv context
          for (const auto& block : payloads.alt.atv.context) {
            if (!vbk.acceptBlock(block, state)) {
              return state.addStackFunction("AltTree::addPayloads");
            }
          }
        }

        return true;
      },
      [&]() { removePayloads(tree, payloads); });
}

template <>
void removePayloads(AltTree& tree, const Payloads& payloads) {
  auto& vbk = tree.vbk();
  removePayloads(tree.vbk(), payloads);

  /// first, remove ATV context
  if (payloads.alt.hasAtv) {
    for (const auto& b : payloads.alt.atv.context) {
      vbk.invalidateBlockByHash(b.getHash());
    }
  }

  /// second, remove VTBs context
  for (const auto& vtb : payloads.vtbs) {
    for (const auto& b : vtb.context) {
      vbk.invalidateBlockByHash(b.getHash());
    }
  }

  /// third, remove vbk context
  for (const auto& b : payloads.vbkcontext) {
    vbk.invalidateBlockByHash(b.getHash());
  }
}

}  // namespace altintegration