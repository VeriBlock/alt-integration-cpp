#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/storage/payloads_repository.hpp>

namespace altintegration {

/// @invariant NOT atomic - given a block tree, if any of functions fail, state
/// is NOT changed back. It you care about the state of your tree, COPY your
/// block tree and use copy in this state machine here.
template <typename ProtectingBlockTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
struct PopStateMachine {
  using index_t = ProtectedIndex;
  using payloads_t = typename ProtectedIndex::payloads_t;
  using height_t = typename ProtectedIndex::height_t;
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  PopStateMachine(ProtectingBlockTree& tree,
                  ProtectedIndex* index,
                  const ProtectedChainParams& protectedParams,
                  height_t startHeight = 0)
      : index_(index),
        tree_(tree),
        protectedParams_(protectedParams),
        startHeight_(startHeight) {}

  //! @invariant: atomic. Either all 'payloads' added or not at all.
  bool applyContext(const index_t&, ValidationState&);

  //! @invariant: atomic. Does not throw under normal conditions.
  void unapplyContext(const index_t&);

  //! does validation of endorsement inside payloads.
  //! assumes context has been applied
  bool addPayloads(const payloads_t& p, ValidationState& state) {
    // endorsement validity window
    auto window = params().getEndorsementSettlementInterval();
    auto minHeight = index_->height >= window ? index_->height - window : 0;
    Chain<BlockIndex<VbkBlock>> chain(minHeight, index_);

    auto endorsedHeight = p.transaction.publishedBlock.height;
    assert(index_->height > endorsedHeight);
    if (index_->height - endorsedHeight > window) {
      return state.Invalid(
          "addPayloadsToBlockIndex", "expired", "Endorsement expired");
    }

    auto* endorsed = chain[endorsedHeight];
    if (!endorsed) {
      return state.Invalid("addPayloadsToBlockIndex",
                           "no-endorsed-block",
                           "No block found on endorsed block height");
    }

    if (endorsed->getHash() != p.transaction.publishedBlock.getHash()) {
      return state.Invalid("addPayloadsToBlockIndex",
                           "block-differs",
                           "Endorsed VBK block is on a different chain");
    }

    auto endorsement = endorsement_t::fromContainer(p);
    auto* blockOfProof = tree_.getBlockIndex(endorsement.blockOfProof);
    if (!blockOfProof) {
      return state.Invalid("addPayloads",
                           "block-of-proof-not-found",
                           "Can not find block of proof in BTC");
    }

    auto* duplicate = chain.findBlockContainingEndorsement(endorsement, window);
    if (duplicate) {
      // found duplicate
      return state.Invalid("addPayloadsToBlockIndex",
                           "duplicate",
                           "Found duplicate endorsement on the same chain");
    }

    auto pair = index_->containingEndorsements.insert(
        {endorsement.id, std::make_shared<BtcEndorsement>(endorsement)});
    assert(pair.second && "there's a duplicate in endorsement map");

    auto* eptr = pair.first->second.get();
    endorsed->endorsedBy.push_back(eptr);

    return true;
  }

  //! the opposite function to 'addPayloads'
  void removePayloads(const payloads_t& payloads) {
    // here we need to remove this endorsement from 'endorsedBy',
    // 'containingPayloads' and 'containingEndorsements'

    // remove from 'containing payloads'
    auto& p = index_->containingPayloads;
    p.erase(std::remove(p.begin(), p.end(), payloads.getId()), p.end());

    // remove from 'endorsedBy'
    auto eid = endorsement_t::getId(payloads);
    auto endorsedHeight = payloads.transaction.publishedBlock.height;
    auto* endorsed = index_->getAncestor(endorsedHeight);
    assert(endorsed);

    auto endorsementit = index_->containingEndorsements.find(eid);
    assert(endorsementit != index_->containingEndorsements.end());
    auto& endorsement = endorsementit->second;

    auto& e = endorsed->endorsedBy;
    e.erase(std::remove_if(e.begin(),
                           e.end(),
                           [&endorsement](const endorsement_t* e) {
                             // remove nullptrs and our given endorsement
                             return !e || endorsement.get() == e;
                           }),
            e.end());

    // remove from 'containing endorsements'
    index_->containingEndorsements.erase(endorsementit);

    removeContextFromBlockIndex(index_, p);
  }

  void unapply(ProtectedIndex& to) {
    if (&to == index_) {
      // already at this state
      return;
    }

    Chain<ProtectedIndex> chain(startHeight_, index_);
    auto* forkPoint = chain.findFork(&to);
    auto* current = chain.tip();
    while (current && current != forkPoint) {
      // unapply payloads
      unapplyContext(*current);
      current = current->pprev;
      index_ = current;
    }

    assert(index_);
    assert(index_ == forkPoint);
  }

  bool apply(ProtectedIndex& to, ValidationState& state) {
    if (&to == index_) {
      // already at this state
      return true;
    }

    Chain<ProtectedIndex> fork(startHeight_, &to);
    auto* current = const_cast<ProtectedIndex*>(fork.findFork(index_));
    assert(current);

    // move forward from forkPoint to "to" and apply payloads in between

    // exclude fork point itself
    current = fork.next(current);

    while (current) {
      if (!applyContext(*current, state)) {
        return state.addStackFunction("PopAwareForkResolution::apply");
      }

      index_ = current;

      if (current != &to) {
        current = fork.next(current);
      } else {
        break;
      }
    }

    assert(index_ == &to);

    return true;
  }

  bool unapplyAndApply(ProtectedIndex& to, ValidationState& state) {
    if (&to == index_) {
      // already at this state
      return true;
    }

    unapply(to);
    return apply(to, state);
  }

  ProtectedIndex* index() { return index_; }
  const ProtectedIndex* index() const { return index_; }
  ProtectingBlockTree& tree() { return tree_; }
  const ProtectingBlockTree& tree() const { return tree_; }
  const ProtectedChainParams& params() const { return protectedParams_; }

 private:
  ProtectedIndex* index_;
  ProtectingBlockTree& tree_;
  const ProtectedChainParams& protectedParams_;

  height_t startHeight_ = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
