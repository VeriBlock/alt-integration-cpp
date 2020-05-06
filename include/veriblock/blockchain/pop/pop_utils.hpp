// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_

#include <algorithm>
#include <unordered_set>

#include "veriblock/blockchain/chain.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

template <typename ProtectingBlockTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
bool checkAndAddEndorsement(
    ProtectedIndex& index,
    const typename ProtectedIndex::endorsement_t& endorsement,
    const ProtectingBlockTree& tree,
    const ProtectedChainParams& params,
    ValidationState& state) {
  using endorsement_t = typename ProtectedIndex::endorsement_t;
  // endorsement validity window
  auto window = params.getEndorsementSettlementInterval();
  auto minHeight = index.height >= window ? index.height - window : 0;
  Chain<ProtectedIndex> chain(minHeight, &index);

  auto endorsedHeight = endorsement.endorsedHeight;
  if (index.height - endorsedHeight > window) {
    return state.Invalid("expired", "Endorsement expired");
  }

  auto* endorsed = chain[endorsedHeight];
  if (!endorsed) {
    return state.Invalid("no-endorsed-block",
                         "No block found on endorsed block height");
  }

  if (endorsed->getHash() != endorsement.endorsedHash) {
    return state.Invalid("block-differs",
                         "Endorsed block is on a different chain");
  }

  auto* blockOfProof = tree.getBlockIndex(endorsement.blockOfProof);
  if (!blockOfProof) {
    return state.Invalid("block-of-proof-not-found",
                         "Can not find block of proof in BTC");
  }

  if (endorsement_t::checkForDuplicates) {
    auto* duplicate = chain.findBlockContainingEndorsement(endorsement, window);
    if (duplicate) {
      // found duplicate
      return state.Invalid("duplicate",
                           "Found duplicate endorsement on the same chain");
    }
  }

  // Add endorsement into BlockIndex
  auto pair = index.containingEndorsements.insert(
      {endorsement.id, std::make_shared<endorsement_t>(endorsement)});
  if (pair.second) {
    auto* eptr = pair.first->second.get();
    endorsed->endorsedBy.push_back(eptr);
  }

  return true;
}

template <typename ProtectedIndex>
void removeEndorsement(
    ProtectedIndex& index,
    const typename ProtectedIndex::endorsement_t::id_t& eid) {
  auto endorsementit = index.containingEndorsements.find(eid);
  if (endorsementit == index.containingEndorsements.end()) {
    return;
  }

  auto& endorsement_ptr = endorsementit->second;

  // remove from 'endorsedBy'
  removeFromEndorsedBy(index, endorsement_ptr.get());
  // remove from 'containing endorsements'
  index.containingEndorsements.erase(endorsementit);
}

template <typename ProtectedIndex>
void removeAllContainingEndorsements(ProtectedIndex& index) {
  for (auto it = index.containingEndorsements.begin();
       it != index.containingEndorsements.end();) {
    removeFromEndorsedBy(index, it->second.get());
    it = index.containingEndorsements.erase(it);
  }
}

template <typename ProtectedIndex>
void removeFromEndorsedBy(
    ProtectedIndex& index,
    const typename ProtectedIndex::endorsement_t* endorsement) {
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  auto endorsed = index.getAncestor(endorsement->endorsedHeight);
  if (endorsed) {
    auto& endorsements = const_cast<ProtectedIndex*>(endorsed)->endorsedBy;
    auto new_end = std::remove_if(endorsements.begin(),
                                  endorsements.end(),
                                  [&endorsement](endorsement_t* e) -> bool {
                                    // remove nullptrs and our given
                                    // endorsement
                                    return !e || endorsement == e;
                                  });

    endorsements.erase(new_end, endorsements.end());
  }
}

template <typename BlockType, typename BlockTreeType>
void addBlockIfUnique(
    const BlockType& block,
    std::unordered_set<typename BlockType::hash_t>& known_blocks,
    std::vector<std::shared_ptr<BlockType>>& vec,
    const BlockTreeType& tree) {
  typename BlockType::hash_t hash = block.getHash();
  // filter context: add only blocks that are unknown and not in current
  // 'known_blocks' if we inserted into known_blocks and tree does not know
  // about this block
  if (known_blocks.insert(hash).second && tree.getBlockIndex(hash) == nullptr) {
    vec.push_back(std::make_shared<BlockType>(block));
  }
}

template <typename BlockType, typename BlockTreeType>
void addBlockIfUnique(
    const std::shared_ptr<BlockType>& block_ptr,
    std::unordered_set<typename BlockType::hash_t>& known_bocks,
    std::vector<std::shared_ptr<BlockType>>& vec,
    const BlockTreeType& tree) {
  typename BlockType::hash_t hash = block_ptr->getHash();
  // filter context: add only blocks that are unknown and not in current
  // 'known_blocks' if we inserted into known_blocks and tree does not know
  // about this block
  if (known_bocks.insert(hash).second && tree.getBlockIndex(hash) == nullptr) {
    vec.push_back(block_ptr);
  }
}

}  // namespace altintegration

#endif
