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
  using storage_t = PayloadsStorage;

  PopStateMachine(ProtectedTree& ed,
                  ProtectingBlockTree& ing,
                  storage_t& storage,
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
    VBK_ASSERT(index.isValid() &&
               "we should have returned earlier when block is invalid");

    auto containingHash = index.getHash();
    if (!index.payloadsIdsEmpty()) {
      std::vector<const CommandGroup*> executed;
      auto cgroups = storage_.loadCommands<ProtectedTree>(index, ed_);
      // even if the block is marked as invalid, we still try to apply it
      for (const auto& cgroup : cgroups) {
        VBK_LOG_DEBUG("Applying payload %s from block %s",
                      HexStr(cgroup.id),
                      index.toShortPrettyString());

        if (cgroup.execute(state)) {
          executed.emplace_back(&cgroup);
          // we were able to apply the command group, so flag it as valid,
          // unless we are in in 'continueOnInvalid' mode which precludes
          // payload re-validation
          if (!continueOnInvalid_) {
            storage_.setValidity(containingHash, cgroup.id, true);
          }
        } else {
          // flag the command group as invalid
          storage_.setValidity(containingHash, cgroup.id, false);

          if (continueOnInvalid_) {
            removePayloadsFromIndex<block_t>(storage_, index, cgroup);
            state.clear();

          } else {
            cleanupAll(index, executed, state);

            // if the block is marked as valid, invalidate its subtree
            if (index.isValid()) {
              ed_.invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
            }

            return state.Invalid(index_t::block_t::name() + "-bad-command");
          }
        }

      }  // end for

      // we have successfully applied the block
      // if the block is marked as invalid, revalidate its subtree
      if (!index.isValid()) {
        ed_.revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
      }
    } else {
      // an empty block must be valid
      VBK_ASSERT(index.isValid());
    }

    index.setFlag(BLOCK_APPLIED);
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
      auto cgroups = storage_.loadCommands<ProtectedTree>(index, ed_);
      for (const auto& cgroup : reverse_iterate(cgroups)) {
        VBK_LOG_DEBUG("Unapplying payload %s from block %s",
                      HexStr(cgroup.id),
                      index.toShortPrettyString());
        cgroup.unExecute();
      }
    }

    index.unsetFlag(BLOCK_APPLIED);
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
    if (&from == &to) {
      // already applied this block
      return true;
    }

    // return early, if 'to' is invalid
    if (!to.isValid()) {
      return false;
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
    if (&from == &to) {
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
  void cleanupAll(index_t& index,
                  const std::vector<const CommandGroup*>& executed,
                  ValidationState& state) {
    VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                  index_t::block_t::name(),
                  index.toPrettyString(),
                  state.toString());

    // unexecute executed commands in the reverse order
    for (auto* group : reverse_iterate(executed)) {
      group->unExecute();
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
