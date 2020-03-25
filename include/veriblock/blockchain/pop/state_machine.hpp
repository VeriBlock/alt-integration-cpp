#ifndef ALTINTEGRATION_STATE_MACHINE_HPP
#define ALTINTEGRATION_STATE_MACHINE_HPP

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
struct BlockTreeStateMachine {
  using payloads_t = typename ProtectedIndex::payloads_t;
  using height_t = typename ProtectedIndex::height_t;
  using get_payloads_f =
      std::function<std::vector<typename ProtectedIndex::payloads_t>(
          const ProtectedIndex& index)>;

  BlockTreeStateMachine(ProtectingBlockTree& tree,
                        ProtectedIndex* index,
                        const ProtectedChainParams& protectedParams,
                        const PayloadsRepository<payloads_t>& p,
                        height_t startHeight = 0)
      : index_(index),
        tree_(tree),
        protectedParams_(protectedParams),
        p_(p),
        startHeight_(startHeight) {}

  //! @invariant: atomic. Either all 'payloads' added or not at all.
  bool addPayloads(const payloads_t& payloads, ValidationState& state);

  //! @invariant: atomic. Does not throw under normal conditions.
  void removePayloads(const payloads_t& payloads);

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
      for (const auto& payloads : getPayloads(*current)) {
        removePayloads(tree_, payloads);
      }
      index_ = current;
      current = current->pprev;
    }
  }

  bool apply(ProtectedIndex& to, ValidationState& state) {
    if (&to == index_) {
      // already at this state
      return true;
    }

    Chain<ProtectedIndex> fork(startHeight_, &to);

    auto* current = index_;
    // move forward from forkPoint to "to" and apply payloads in between

    // exclude fork point itself
    current = fork.next(current);

    std::vector<payloads_t> applied;
    while (current) {
      for (const auto& payloads : getPayloads(*current)) {
        if (!addPayloads(tree_, payloads, state)) {
          return state.addStackFunction("PopAwareForkResolution::apply");
        }

        *index_ = current;
      }

      if (current != &to) {
        current = fork.next(current);
      } else {
        break;
      }
    }

    assert(*index_ == &to);

    return true;
  }

  bool unapplyAndApply(ProtectedIndex& to, ValidationState& state) {
    if (&to == index_) {
      // already at this state
      return true;
    }

    unapply(to);

    Chain<ProtectedIndex> chain(startHeight_, &to);
    if (chain.contains(*index_)) {
      // do not apply payloads as "to" is in current chain and no new payloads
      // will be added
      return true;
    }

    return apply(to, state);
  }

  ProtectedIndex* index() { return index_; }
  const ProtectedIndex* index() const { return index_; }
  ProtectingBlockTree& tree() { return tree_; }
  const ProtectingBlockTree& tree() const { return tree_; }
  const ProtectedChainParams& params() const { return protectedParams_; }

 private:
  std::vector<payloads_t> getPayloads(const ProtectedIndex& index) {
    std::vector<payloads_t> ret;
    ret.reserve(index.containingPayloads.size());
    p_.get(index.containingPayloads, &ret);
    return ret;
  }

 private:
  ProtectedIndex* index_;
  ProtectingBlockTree& tree_;
  const ProtectedChainParams& protectedParams_;

  const PayloadsRepository<payloads_t>& p_;
  height_t startHeight_ = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_STATE_MACHINE_HPP
