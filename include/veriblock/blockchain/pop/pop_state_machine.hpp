// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

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

  bool applyBlock(index_t& index, ValidationState& state) {
    bool success = true;
    for (size_t groupIndex = 0; groupIndex < index.commands.size(); groupIndex++) {
      auto& group = index.commands[groupIndex];

      for (size_t cmdIndex = 0; cmdIndex < group.commands.size(); cmdIndex++) {
        const auto& cmd = group.commands[cmdIndex];

        if (!cmd->Execute(state)) {
          VBK_LOG_WARN("Invalid %s command %s in block %s",
                       index_t::block_t::name(),
                       cmd->toPrettyString(),
                       index.toPrettyString());
          success = false;

          // roll back the slice of the group that has already been executed
          for (size_t rollbackCmdIndex = cmdIndex; rollbackCmdIndex > 0; rollbackCmdIndex--) {
              group.commands[rollbackCmdIndex - 1]->UnExecute();
          }
          break;
        }
      }

      if (!success) {
        // roll back the groups that have already been executed
        for (size_t rollbackGroupIndex = groupIndex; rollbackGroupIndex > 0; rollbackGroupIndex--) {
          const auto& rollbackGroup = index.commands[rollbackGroupIndex - 1];
          std::for_each(rollbackGroup.rbegin(), rollbackGroup.rend(), [](const CommandPtr& cmd) {
            cmd->UnExecute();
          });
        }
        break;
      }
    }

    if (!success) {
      state.Invalid(index_t::block_t::name() + "-bad-command");
    }
    return success;
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

    VBK_LOG_DEBUG("Unapply %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

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

    VBK_LOG_DEBUG("Applying %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* index : chain) {
      if (!index->isValid() || !applyBlock(*index, state)) {
        unapply(*index->pprev, from);
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
