// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/assert.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/reversed_range.hpp>
#include <veriblock/storage/payloads_storage.hpp>

namespace altintegration {

template <typename ProtectingBlockTree,
          typename ProtectedTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
struct PopStateMachine {
  using index_t = ProtectedIndex;
  using block_t = typename index_t::block_t;
  using endorsement_t = typename index_t::endorsement_t;
  using height_t = typename ProtectedIndex::height_t;

  PopStateMachine(ProtectedTree& ed,
                  ProtectingBlockTree& ing,
                  PayloadsStorage& storage,
                  height_t startHeight = 0,
                  bool continueOnInvalid = false)
      : ed_(ed),
        ing_(ing),
        storage_(storage),
        startHeight_(startHeight),
        continueOnInvalid_(continueOnInvalid) {}

  // atomic: applies either all or none of the block's commands
  bool applyBlock(index_t& index, ValidationState& state) {
    VBK_ASSERT(index.pprev && "cannot apply the genesis block");
    VBK_ASSERT(index.pprev->hasFlags(BLOCK_APPLIED) &&
               "state corruption: tried to apply a block that follows an "
               "unapplied block");
    VBK_ASSERT(!index.hasFlags(BLOCK_APPLIED) &&
               "state corruption: tried to apply an already applied block");

    if (!index.payloadsIdsEmpty()) {
      std::vector<std::vector<CommandPtr>> executed;
      auto cgs = storage_.loadCommands<ProtectedTree>(index, ed_);
      // even if the block is marked as invalid, we still try to apply it
      for (const auto& cg : cgs) {
        // alloc new vector<CommandPtr> for this command group
        executed.emplace_back();
        VBK_LOG_DEBUG("Applying payload %s from block %s",
                      HexStr(cg.id),
                      index.toShortPrettyString());

        // execute commands in this command group
        for (auto& cmd : cg.commands) {
          if (cmd->Execute(state)) {
            // command is valid
            executed.back().push_back(cmd);
            continue;
          }

          // command is invalid
          storage_.setValidity(cg, index, false);
          if (continueOnInvalid_) {
            cleanupLast(index, executed, cg);
            state.clear();
            break;  // end cmd loop, start from next group
          } else {
            cleanupAll(index, executed, state);

            // if the block is marked as valid, invalidate its subtree
            if (index.isValid()) {
              ed_.invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
            }

            return state.Invalid(index_t::block_t::name() + "-bad-command");
          }
        }  // end for

        // in 'continueOnInvalid' mode, we don't re-validate payloads
        if (!continueOnInvalid_) {
          // continueOnInvalid=false and we were able to apply given
          // CommandGroup. It means that it is valid, so update its validity.
          storage_.setValidity(cg, index, true);
        }
      }  // end for

      // we successfully applied the block
      // if the block is marked as invalid, revalidate its subtree
      if (!index.isValid()) {
        ed_.revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
      }
    } else {
      // an empty block must be valid
      VBK_ASSERT(index.isValid());
    }

    index.setFlagSetDirty(BLOCK_APPLIED);
    return true;
  }

  // atomic: applies either all of the block's commands or fails on an assert
  void unapplyBlock(index_t& index) {
    // this check is expensive; might want to eventually disable it
    bool allLeavesUnapplied =
        std::all_of(index.pnext.begin(), index.pnext.end(), [](index_t* index) {
          return !index->hasFlags(BLOCK_APPLIED);
        });
    VBK_ASSERT(allLeavesUnapplied &&
               "state corruption: tried to unapply a block before unapplying "
               "its applied leaves");

    VBK_ASSERT(index.hasFlags(BLOCK_APPLIED) &&
               "state corruption: tried to unapply an already unapplied block");
    VBK_ASSERT(index.pprev && "cannot unapply the genesis block");
    VBK_ASSERT(index.pprev->hasFlags(BLOCK_APPLIED) &&
               "state corruption: tried to unapply a block that follows an "
               "unapplied block");

    if (!index.payloadsIdsEmpty()) {
      auto cgs = storage_.loadCommands<ProtectedTree>(index, ed_);
      for (const auto& cg : reverse_iterate(cgs)) {
        VBK_LOG_DEBUG("Unapplying payload %s from block %s",
                      HexStr(cg.id),
                      index.toShortPrettyString());
        for (auto& cmd : reverse_iterate(cg.commands)) {
          cmd->UnExecute();
        }
      }
    }

    index.unsetFlagSetDirty(BLOCK_APPLIED);
  }

  // unapplies all commands commands from blocks in the range of [from; to)
  // atomic: either applies all of the requested blocks or fails on an assert
  void unapply(index_t& from, index_t& to) {
    if (&from == &to) {
      return;
    }

    VBK_ASSERT(from.getHeight() > to.getHeight());
    // exclude 'to' by adding 1
    Chain<index_t> chain(to.getHeight() + 1, &from);
    VBK_ASSERT(chain.first());
    VBK_ASSERT(chain.first()->pprev == &to);

    VBK_LOG_DEBUG("Unapply %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* current : reverse_iterate(chain)) {
      unapplyBlock(*current);
    }
  }

  // applies all commands from blocks in the range of (from; to].
  // atomic: applies either all or none of the requested blocks
  bool apply(index_t& from, index_t& to, ValidationState& state) {
    if (from == to) {
      // already applied this block
      return true;
    }

    VBK_ASSERT(from.getHeight() < to.getHeight());
    // exclude 'from' by adding 1
    Chain<index_t> chain(from.getHeight() + 1, &to);
    VBK_ASSERT(chain.first());
    VBK_ASSERT(chain.first()->pprev == &from);

    VBK_LOG_DEBUG("Applying %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* index : chain) {
      if (VBK_UNLIKELY(IsShutdownRequested())) {
        return true;
      }

      if (!applyBlock(*index, state)) {
        // rollback the previously appled slice of the chain
        unapply(*index->pprev, from);
        return false;
      }
    }

    // this subchain is valid
    return true;
  }

  // effectively unapplies [from; genesis) and applies (genesis; to]
  // assumes and requires that [from; genesis) is applied
  // optimization: avoids applying/unapplying (genesis; fork_point(from, to)]
  // atomic: either changes the state to 'to' or leaves it unchanged
  bool setState(index_t& from, index_t& to, ValidationState& state) {
    if (VBK_UNLIKELY(IsShutdownRequested())) {
      return true;
    }

    if (from == to) {
      // already at this state
      return true;
    }

    // is 'to' a successor?
    if (to.getAncestor(from.getHeight()) == &from) {
      return apply(from, to, state);
    }

    // 'to' is a predecessor or another fork
    Chain<index_t> chain(startHeight_, &from);
    auto* forkBlock = chain.findFork(&to);

    VBK_ASSERT(forkBlock &&
               "state corruption: from and to must be part of the same tree");

    unapply(from, *forkBlock);
    if (!apply(*forkBlock, to, state)) {
      // attempted to switch to an invalid block, rollback
      bool ret = apply(*forkBlock, from, state);
      VBK_ASSERT(ret);

      return false;
    }

    return true;
  }

  ProtectingBlockTree& tree() { return ing_; }
  const ProtectingBlockTree& tree() const { return ing_; }
  const ProtectedChainParams& params() const { return ed_.getParams(); }

 private:
  void cleanupLast(index_t& index,
                   std::vector<std::vector<CommandPtr>>& executed,
                   const CommandGroup& cg) {
    VBK_ASSERT(!executed.empty());
    // this command group is invalid, unapply commands from this command
    // group
    for (auto& c : reverse_iterate(executed.back())) {
      c->UnExecute();
    }

    // we don't want to unapply same commands twice
    executed.pop_back();

    removePayloadsFromIndex<block_t>(index, cg);
  }

  void cleanupAll(index_t& index,
                  const std::vector<std::vector<CommandPtr>>& executed,
                  ValidationState& state) {
    VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                  index_t::block_t::name(),
                  index.toPrettyString(),
                  state.toString());

    // unexecute executed commands in reverse order
    for (auto& group : reverse_iterate(executed)) {
      for (auto& c : reverse_iterate(group)) {
        c->UnExecute();
      }
    }
  }

 private:
  ProtectedTree& ed_;
  ProtectingBlockTree& ing_;
  PayloadsStorage& storage_;
  height_t startHeight_;
  bool continueOnInvalid_ = false;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
