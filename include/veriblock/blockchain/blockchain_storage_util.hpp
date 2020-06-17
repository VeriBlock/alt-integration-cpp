// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_

#include <veriblock/storage/pop_storage.hpp>

namespace altintegration {

template <typename BlockTree>
void saveBlocks(PopStorage& storage, const BlockTree& tree) {
  storage.saveBlocks(tree.getBlocks());
  storage.saveTip(*tree.getBestChain().tip());
}

template <typename BlockTree>
bool loadBlocks(const PopStorage& storage,
                BlockTree& tree,
                ValidationState& state) {
  using block_t = typename BlockTree::block_t;
  using endorsement_t = typename block_t::endorsement_t;
  auto blocks = storage.loadBlocks<typename BlockTree::index_t>();
  auto tipStored = storage.loadTip<typename BlockTree::index_t>();
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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_STORAGE_UTIL_HPP_
