// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/reversed_range.hpp>

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
    std::vector<CommandPtr> executed;
    for (auto it = index.payloadIds.begin(); it != index.payloadIds.end();
         ++it) {
      payloads_t payload{};
      bool ret = ed_.getStorage().payloads().get(*it, &payload);
      if (!ret) continue;
      std::vector<CommandPtr> commands{};
      ed_.payloadsToCommands(payload, commands);

      for (auto& cmd : commands) {
        if (!cmd->Execute(state)) {
          // invalidate command group
          // cg.valid = false;
          VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                        index_t::block_t::name(),
                        index.toPrettyString(),
                        state.toString());

          // unexecute executed commands in reverse order
          std::for_each(executed.rbegin(),
                        executed.rend(),
                        [](const CommandPtr& c) { c->UnExecute(); });

          return state.Invalid(index_t::block_t::name() + "-bad-command");
        }  // end if

        // command is valid
        executed.push_back(cmd);
      }
    }
    return true;
  }

  void unapplyBlock(const index_t& index) {
    for (auto it = index.payloadIds.rbegin(); it != index.payloadIds.rend();
         ++it) {
      payloads_t payload{};
      bool ret = ed_.getStorage().payloads().get(*it, &payload);
      if (!ret) continue;
      std::vector<CommandPtr> commands{};
      ed_.payloadsToCommands(payload, commands);

      VBK_LOG_DEBUG("Unapplying payload %s from block %s",
                    payload.getId().toHex(),
                    index.toShortPrettyString());
      std::for_each(commands.rbegin(),
                    commands.rend(),
                    [](const CommandPtr& cmd) { cmd->UnExecute(); });
    }
  }

  // unapplies commands in range [from; to)
  void unapply(ProtectedIndex& from, ProtectedIndex& to) {
    if (&from == &to) {
      return;
    }

    VBK_ASSERT(from.height > to.height);
    // exclude 'to' by adding 1
    Chain<ProtectedIndex> chain(to.height + 1, &from);
    VBK_ASSERT(chain.first());
    VBK_ASSERT(chain.first()->pprev == &to);

    VBK_LOG_DEBUG("Unapply %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    std::for_each(
        chain.rbegin(), chain.rend(), [&](const ProtectedIndex* current) {
          if (current->payloadIds.empty()) {
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

    VBK_ASSERT(from.height < to.height);
    // exclude 'from' by adding 1
    Chain<ProtectedIndex> chain(from.height + 1, &to);
    VBK_ASSERT(chain.first());
    VBK_ASSERT(chain.first()->pprev == &from);

    VBK_LOG_DEBUG("Applying %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* index : chain) {
      // even if block is invalid, still try to apply it
      if (!applyBlock(*index, state)) {
        unapply(*index->pprev, from);
        // if block is valid, then invalidate it
        if (index->isValid()) {
          ed_.invalidateSubtree(*index, BLOCK_FAILED_POP, /*do fr=*/false);
        }
        return false;
      }

      // we successfully applied block

      // if block is marked as invalid, but were able to successfully apply
      // it, revalidate its subtree
      if (!index->isValid()) {
        ed_.revalidateSubtree(*index, BLOCK_FAILED_POP, /*do fr=*/false);
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
};  // namespace altintegration

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
