#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_

#include <algorithm>

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
                         "Endorsed VBK block is on a different chain");
  }

  auto* blockOfProof = tree.getBlockIndex(endorsement.blockOfProof);
  if (!blockOfProof) {
    return state.Invalid("block-of-proof-not-found",
                         "Can not find block of proof in BTC");
  }

  auto* duplicate = chain.findBlockContainingEndorsement(endorsement, window);
  if (duplicate) {
    // found duplicate
    return state.Invalid("duplicate",
                         "Found duplicate endorsement on the same chain");
  }

  // Add endorsement into BlockIndex
  auto pair = index.containingEndorsements.insert(
      {endorsement.id, std::make_shared<endorsement_t>(endorsement)});
  assert(pair.second && "there's a duplicate in endorsement map");

  auto* eptr = pair.first->second.get();
  endorsed->endorsedBy.push_back(eptr);

  return true;
}

template <typename ProtectedIndex>
void removeEndorsements(
    ProtectedIndex& index,
    const typename ProtectedIndex::endorsement_t& endorsement) {
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  // remove from 'endorsedBy'
  auto endorsed = index.getAncestor(endorsement.endorsedHeight);

  if (endorsed) {
    auto endorsementit = index.containingEndorsements.find(endorsement.id);
    if (endorsementit != index.containingEndorsements.end()) {
      auto& endorsement_ptr = endorsementit->second;

      auto& endorsements = const_cast<ProtectedIndex*>(endorsed)->endorsedBy;
      auto new_end =
          std::remove_if(endorsements.begin(),
                         endorsements.end(),
                         [&endorsement_ptr](endorsement_t* e) -> bool {
                           // remove nullptrs and our given
                           // endorsement
                           return !e || endorsement_ptr.get() == e;
                         });

      endorsements.erase(new_end, endorsements.end());

      // remove from 'containing endorsements'
      index.containingEndorsements.erase(endorsementit);
    }
  }
}

}  // namespace altintegration

#endif
