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
#include <veriblock/storage/payloads_index.hpp>
#include <veriblock/storage/payloads_provider.hpp>

namespace altintegration {

namespace {

template <typename index_t>
void assertBlockCanBeApplied(index_t& index) {
  VBK_ASSERT(index.pprev && "cannot apply the genesis block");

  VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_APPLIED),
                 "state corruption: tried to apply a block that follows an "
                 "unapplied block %s",
                 index.pprev->toPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_APPLIED),
                 "state corruption: tried to apply an already applied block %s",
                 index.toPrettyString());
  // an expensive check; might want to  disable it eventually
  VBK_ASSERT_MSG(
      index.allDescendantsUnapplied(),
      "state corruption: found an unapplied block that has some of its "
      "descendants applied %s",
      index.toPrettyString());

  VBK_ASSERT(!index.hasFlags(BLOCK_FAILED_CHILD) &&
             "state corruption: attempted to apply a block that has an "
             "invalid ancestor");
}

template <typename index_t>
void assertBlockCanBeUnapplied(index_t& index) {
  VBK_ASSERT(index.pprev && "cannot unapply the genesis block");

  VBK_ASSERT_MSG(
      index.hasFlags(BLOCK_APPLIED),
      "state corruption: tried to unapply an already unapplied block %s",
      index.toPrettyString());
  VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_APPLIED),
                 "state corruption: tried to unapply a block that follows an "
                 "unapplied block %s",
                 index.pprev->toPrettyString());
  // an expensive check; might want to  disable it eventually
  VBK_ASSERT_MSG(index.allDescendantsUnapplied(),
                 "state corruption: tried to unapply a block before unapplying "
                 "its applied descendants %s",
                 index.toPrettyString());
}

}  // namespace

//! @private
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
                  PayloadsProvider& payloadsProvider,
                  PayloadsIndex& payloadsIndex,
                  height_t startHeight = 0,
                  bool continueOnInvalid = false)
      : ed_(ed),
        ing_(ing),
        payloadsProvider_(payloadsProvider),
        payloadsIndex_(payloadsIndex),
        startHeight_(startHeight),
        continueOnInvalid_(continueOnInvalid) {}

  // atomic: applies either all or none of the block's commands
  VBK_CHECK_RETURN bool applyBlock(index_t& index, ValidationState& state) {
    assertBlockCanBeApplied(index);

    if (!index.isValid()) {
      return state.Invalid(
          index_t::block_t::name() + "-marked-invalid",
          fmt::sprintf("block %s is marked as invalid and cannot be applied",
                       index.toPrettyString()));
    }

    VBK_ASSERT_MSG(index.isValid(BLOCK_CONNECTED),
                   "attempted to apply an unconnected block %s",
                   index.toPrettyString());

    if (index.hasPayloads()) {
      std::vector<CommandGroup> cgroups;
      bool ret = payloadsProvider_.getCommands(ed_, index, cgroups, state);
      VBK_ASSERT_MSG(ret,
                     "failed to load commands from block=%s, reason=%s",
                     index.toPrettyString(),
                     state.toString());

      const auto containingHash = index.getHash();
      for (auto cgroup = cgroups.cbegin(); cgroup != cgroups.cend(); ++cgroup) {
        VBK_LOG_DEBUG("Applying payload %s from block %s",
                      HexStr(cgroup->id),
                      index.toShortPrettyString());

        if (cgroup->execute(state)) {
          // we were able to apply the command group, so flag it as valid,
          // unless we are in in 'continueOnInvalid' mode which precludes
          // payload re-validation
          if (!continueOnInvalid_) {
            payloadsIndex_.setValidity(containingHash, cgroup->id, true);
          }

        } else {
          // flag the command group as invalid
          payloadsIndex_.setValidity(containingHash, cgroup->id, false);

          if (continueOnInvalid_) {
            removePayloadsFromIndex<block_t>(payloadsIndex_, index, *cgroup);
            state.clear();
            continue;
          }

          VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                        index_t::block_t::name(),
                        index.toPrettyString(),
                        state.toString());

          // unexecute executed command groups in the reverse order
          for (auto r_group = std::reverse_iterator<decltype(cgroup)>(cgroup);
               r_group != cgroups.rend();
               ++r_group) {
            r_group->unExecute();
          }

          ed_.invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);

          return state.Invalid(index_t::block_t::name() + "-bad-command");
        }

      }  // end for
    }

    // if the applied block count equals the size of the chain between the root
    // and index, we have just applied a block on top of the only applied chain,
    // so it is fully valid
    if (index.pprev->isValid(BLOCK_CAN_BE_APPLIED) &&
        index.getHeight() ==
            ed_.getRoot().getHeight() + ed_.appliedBlockCount) {
      index.raiseValidity(BLOCK_CAN_BE_APPLIED);
    } else {
      index.raiseValidity(BLOCK_HAS_BEEN_APPLIED);
    }

    index.setFlag(BLOCK_APPLIED);
    ++ed_.appliedBlockCount;

    return true;
  }

  /**
   * Removes all side effects made by this block.
   * @param[in] index block to unapply
   */
  void unapplyBlock(index_t& index) {
    assertBlockCanBeUnapplied(index);

    if (index.hasPayloads()) {
      std::vector<CommandGroup> cgroups;
      ValidationState state;
      bool ret = payloadsProvider_.getCommands(ed_, index, cgroups, state);
      VBK_ASSERT_MSG(ret,
                     "failed to load commands from block=%s, reason=%s",
                     index.toPrettyString(),
                     state.toString());

      for (const auto& cgroup : reverse_iterate(cgroups)) {
        VBK_LOG_DEBUG("Unapplying payload %s from block %s",
                      HexStr(cgroup.id),
                      index.toShortPrettyString());
        cgroup.unExecute();
      }
    }

    index.unsetFlag(BLOCK_APPLIED);
    VBK_ASSERT(ed_.appliedBlockCount > 0);
    --ed_.appliedBlockCount;
  }

  // unapplies all commands commands from blocks in the range of [from; to)
  // while predicate returns true, if predicate return false stop unapplying and
  // return the index on which predicate returns false
  // atomic: either applies all of the requested blocks or fails on an assert
  VBK_CHECK_RETURN index_t* unapplyWhile(
      index_t& from,
      index_t& to,
      const std::function<bool(index_t& index)>& pred) {
    if (&from == &to) {
      return &to;
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
      VBK_ASSERT(current != nullptr);
      if (pred(*current)) {
        unapplyBlock(*current);
      } else {
        return current;
      }
    }

    return &to;
  }

  // unapplies all commands commands from blocks in the range of [from; to)
  // atomic: either applies all of the requested blocks
  // or fails on an assert
  void unapply(index_t& from, index_t& to) {
    auto pred = [](index_t&) -> bool { return true; };
    auto* index = unapplyWhile(from, to, pred);
    VBK_ASSERT(index == &to);
  }

  // applies all commands from blocks in the range of (from; to].
  // atomic: applies either all or none of the requested blocks
  VBK_CHECK_RETURN bool apply(index_t& from,
                              index_t& to,
                              ValidationState& state) {
    if (&from == &to) {
      // already applied this block
      return true;
    }

    if (!to.isValid()) {
      return state.Invalid(
          index_t::block_t::name() + "-marked-invalid",
          fmt::sprintf("block %s is marked as invalid and cannot be applied",
                       to.toPrettyString()));
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
        // rollback the previously applied slice of the chain
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
  VBK_CHECK_RETURN bool setState(index_t& from,
                                 index_t& to,
                                 ValidationState& state) {
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
      bool success = apply(*forkBlock, from, state);
      VBK_ASSERT(success && "state corruption: failed to rollback the state");

      return false;
    }

    return true;
  }

  ProtectingBlockTree& tree() { return ing_; }
  const ProtectingBlockTree& tree() const { return ing_; }
  const ProtectedChainParams& params() const { return ed_.getParams(); }

 private:
  ProtectedTree& ed_;
  ProtectingBlockTree& ing_;
  PayloadsProvider& payloadsProvider_;
  PayloadsIndex& payloadsIndex_;
  height_t startHeight_ = 0;
  bool continueOnInvalid_ = false;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
