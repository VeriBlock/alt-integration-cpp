// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/blockchain_storage_util.hpp"

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"

namespace altintegration {

template <>
bool loadBlocks(const PopStorage& storage,
                AltTree& tree,
                ValidationState& state) {
  using block_t = typename AltTree::block_t;
  using endorsement_t = typename AltTree::index_t::endorsement_t;

  auto blocks = storage.loadBlocks<typename AltTree::index_t>();
  auto tipStored = storage.loadTip<typename AltTree::index_t>();

  for (const auto& blockPair : blocks) {
    auto* bi = tree.insertBlock(blockPair.second->header);
    bi->alt_payloadIds = blockPair.second->alt_payloadIds;
    bi->vbk_payloadIds = blockPair.second->vbk_payloadIds;
    bi->vbk_blockIds = blockPair.second->vbk_blockIds;

    // load endorsements
    for (const auto& e : blockPair.second->containingEndorsements) {
      auto endorsement = storage.loadEndorsements<endorsement_t>(e.first);
      auto* endorsed = tree.template getBlockIndex<typename block_t::hash_t>(
          endorsement.endorsedHash);
      if (endorsed == nullptr) {
        return state.Invalid(
            block_t::name() + "-bad-endorsed",
            "Can not find endorsed block: " + HexStr(endorsement.endorsedHash));
      }
      auto endorsementPtr =
          std::make_shared<endorsement_t>(std::move(endorsement));
      bi->containingEndorsements.insert(
          std::make_pair(endorsementPtr->getId(), endorsementPtr));
      endorsed->endorsedBy.push_back(endorsementPtr.get());
    }
  }

  auto* tip = tree.getBlockIndex(tipStored.second);
  if (tip == nullptr) return false;
  if (tip->height != tipStored.first) return false;
  return tree.setState(*tip, state, true);
}

template <>
bool loadBlocks(const PopStorage& storage,
                VbkBlockTree& tree,
                ValidationState& state) {
  using block_t = typename VbkBlockTree::block_t;
  using endorsement_t = typename VbkBlockTree::index_t::endorsement_t;

  auto blocks = storage.loadBlocks<typename VbkBlockTree::index_t>();
  auto tipStored = storage.loadTip<typename VbkBlockTree::index_t>();

  for (const auto& blockPair : blocks) {
    auto* bi = tree.insertBlock(blockPair.second->header);
    bi->payloadIds = blockPair.second->payloadIds;
    bi->refCounter = blockPair.second->refCounter;

    // load endorsements
    for (const auto& e : blockPair.second->containingEndorsements) {
      auto endorsement = storage.loadEndorsements<endorsement_t>(e.first);
      auto* endorsed = tree.template getBlockIndex<typename block_t::hash_t>(
          endorsement.endorsedHash);
      if (endorsed == nullptr) {
        return state.Invalid(
            block_t::name() + "-bad-endorsed",
            "Can not find endorsed block: " + HexStr(endorsement.endorsedHash));
      }
      auto endorsementPtr =
          std::make_shared<endorsement_t>(std::move(endorsement));
      bi->containingEndorsements.insert(
          std::make_pair(endorsementPtr->getId(), endorsementPtr));
      endorsed->endorsedBy.push_back(endorsementPtr.get());
    }
  }

  auto* tip = tree.getBlockIndex(tipStored.second);
  if (tip == nullptr) return false;
  if (tip->height != tipStored.first) return false;
  return tree.setState(*tip, state, true);
}

template <>
bool loadBlocks(const PopStorage& storage,
                BlockTree<BtcBlock, BtcChainParams>& tree,
                ValidationState& state) {
  using BtcBlockTree = BlockTree<BtcBlock, BtcChainParams>;

  auto blocks = storage.loadBlocks<typename BtcBlockTree::index_t>();
  auto tipStored = storage.loadTip<typename BtcBlockTree::index_t>();

  for (const auto& blockPair : blocks) {
    auto* bi = tree.insertBlock(blockPair.second->header);
    bi->refCounter = blockPair.second->refCounter;
  }

  auto* tip = tree.getBlockIndex(tipStored.second);
  if (tip == nullptr) return false;
  if (tip->height != tipStored.first) return false;
  return tree.setState(*tip, state, true);
}  // namespace altintegration

}  // namespace altintegration