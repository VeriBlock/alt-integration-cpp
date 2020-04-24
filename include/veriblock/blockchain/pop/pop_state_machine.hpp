// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/finalizer.hpp>
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
        protectedParams_(&protectedParams),
        startHeight_(startHeight) {}

  //! @invariant: atomic. Either whole block is added or not at all.
  bool applyBlock(const index_t& index, ValidationState& state) {
    CommandHistory history;
    size_t i = 0;
    for (const auto& cmd : index.commands) {
      if (!history.exec(cmd, state)) {
        history.undoAll();
        return state.Invalid("bad-command", i);
      }
      i++;
    }
    return true;
  }

  //! @invariant: atomic. Does not throw under normal conditions.
  void unapplyBlock(const index_t& index) {
    auto& v = index.commands;
    std::for_each(
        v.rbegin(), v.rend(), [](const CommandPtr& cmd) { cmd->UnExecute(); });
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
      unapplyBlock(*current);
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

    auto* current = const_cast<index_t*>(fork.findFork(index_));
    assert(current);

    // move forward from forkPoint to "to" and apply payloads in between

    // exclude fork point itself
    current = fork.next(current);

    while (current) {
      if (!applyBlock(*current, state)) {
        return state.Invalid("pop-apply", current->height);
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
  const ProtectedChainParams& params() const { return *protectedParams_; }

 private:
  ProtectedIndex* index_;
  ProtectingBlockTree& tree_;
  const ProtectedChainParams* protectedParams_;

  height_t startHeight_ = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
