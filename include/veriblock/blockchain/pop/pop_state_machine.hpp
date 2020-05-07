// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/storage/payloads_repository.hpp>

namespace altintegration {

template <typename ProtectingBlockTree,
          typename ProtectedTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
struct PopStateMachine {
  using index_t = ProtectedIndex;
  using payloads_t = typename ProtectedIndex::payloads_t;
  using height_t = typename ProtectedIndex::height_t;
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  PopStateMachine(ProtectedTree& ed,
                  ProtectingBlockTree& ing,
                  height_t startHeight = 0)
      : ed_(ed), ing_(ing), startHeight_(startHeight) {}

  bool applyBlock(const index_t& index, ValidationState& state) {
    for (const auto& group : index.commands) {
      for (const auto& cmd : group) {
        if (!cmd->Execute(state)) {
          return false;
        }
      }
    }
    return true;
  }

  void unapplyBlock(const index_t& index) {
    auto& v = index.commands;
    std::for_each(v.rbegin(), v.rend(), [](const CommandGroup& group) {
      std::for_each(group.rbegin(), group.rend(), [](const CommandPtr& cmd) {
        cmd->UnExecute();
      });
    });
  }

  // unapplies commands in range [from; to)
  void unapply(ProtectedIndex& from, ProtectedIndex& to) {
    if (&from == &to) {
      return;
    }

    assert(from.height > to.height);
    // exclude 'to' by adding 1
    Chain<ProtectedIndex> chain(to.height + 1, &from);
    assert(chain.first());
    assert(chain.first()->pprev == &to);

    std::for_each(
        chain.rbegin(), chain.rend(), [&](const ProtectedIndex* current) {
          if (current->commands.empty()) {
            return;
          }

          unapplyBlock(*current);
        });
  }

  // applies commands in range (from; to].
  bool apply(ProtectedIndex& from, ProtectedIndex& to, ValidationState& state) {
    if (from == to) {
      // already applied this block
      return true;
    }

    assert(from.height < to.height);
    // exclude 'from' by adding 1
    Chain<ProtectedIndex> chain(from.height + 1, &to);
    assert(chain.first());
    assert(chain.first()->pprev == &from);

    for (auto* index : chain) {
      if (!index->isValid() || !applyBlock(*index, state)) {
        unapply(*index, from);
        // don't do fork resolution, as it
        ed_.invalidateSubtree(
            *index, BLOCK_FAILED_POP, /* do fork resolution= */ false);
        return false;
      }
    }

    // this subchain is valid
    return true;
  }

  ProtectingBlockTree& tree() { return ing_; }
  const ProtectingBlockTree& tree() const { return ing_; }
  const ProtectedChainParams& params() const { return ed_.getParams(); }

 private:
  ProtectedTree& ed_;
  ProtectingBlockTree& ing_;
  height_t startHeight_ = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
