#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_POP_UTILS_HPP_

#include <algorithm>

#include "veriblock/blockchain/chain.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

template <typename ProtectingBlockTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
bool checkAndAddEndorsement(ProtectedIndex& index,
                            const typename ProtectedIndex::payloads_t& p,
                            const ProtectingBlockTree& tree,
                            const ProtectedChainParams& params,
                            ValidationState& state) {
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  // endorsement validity window
  auto window = params.getEndorsementSettlementInterval();
  auto minHeight = index.height >= window ? index.height - window : 0;
  Chain<ProtectedIndex> chain(minHeight, &index);

  auto endorsedHeight = p.transaction.publishedBlock.height;
  if (index.height - endorsedHeight > window) {
    return state.Invalid("addPayloadsToBlockIndex",
                         "expired : Endorsement expired");
  }

  auto* endorsed = chain[endorsedHeight];
  if (!endorsed) {
    return state.Invalid(
        "addPayloadsToBlockIndex",
        "no-endorsed-block : No block found on endorsed block height");
  }

  if (endorsed->getHash() != p.transaction.publishedBlock.getHash()) {
    return state.Invalid(
        "addPayloadsToBlockIndex",
        "block-differs : Endorsed VBK block is on a different chain");
  }

  auto endorsement = endorsement_t::fromContainer(p);
  auto* blockOfProof = tree.getBlockIndex(endorsement.blockOfProof);
  if (!blockOfProof) {
    return state.Invalid(
        "addPayloads",
        "block-of-proof-not-found : Can not find block of proof in BTC");
  }

  auto* duplicate = chain.findBlockContainingEndorsement(endorsement, window);
  if (duplicate) {
    // found duplicate
    return state.Invalid(
        "addPayloadsToBlockIndex",
        "duplicate : Found duplicate endorsement on the same chain");
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
void removeEndorsements(ProtectedIndex& index,
                        const typename ProtectedIndex::payloads_t& payloads) {
  using endorsement_t = typename ProtectedIndex::endorsement_t;
  // here we need to remove this endorsement from 'endorsedBy',
  // 'containingPayloads' and 'containingEndorsements'

  // remove from 'endorsedBy'
  auto eid = endorsement_t::getId(payloads);
  auto endorsedHeight = payloads.transaction.publishedBlock.height;
  auto endorsed = index.getAncestor(endorsedHeight);

  if (endorsed) {
    auto endorsementit = index.containingEndorsements.find(eid);
    if (endorsementit != index.containingEndorsements.end()) {
      auto& endorsement = endorsementit->second;

      auto& endorsements = const_cast<ProtectedIndex*>(endorsed)->endorsedBy;
      auto new_end = std::remove_if(endorsements.begin(),
                                    endorsements.end(),
                                    [&endorsement](endorsement_t* e) -> bool {
                                      // remove nullptrs and our given
                                      // endorsement
                                      return !e || endorsement.get() == e;
                                    });

      endorsements.erase(new_end, endorsements.end());

      // remove from 'containing endorsements'
      index.containingEndorsements.erase(endorsementit);
    }
  }
}

}  // namespace altintegration

#endif
