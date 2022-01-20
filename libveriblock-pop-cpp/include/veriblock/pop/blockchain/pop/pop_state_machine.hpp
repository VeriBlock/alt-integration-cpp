// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/blockchain/chain.hpp>
#include <veriblock/pop/blockchain/chain_slice.hpp>
#include <veriblock/pop/reversed_range.hpp>
#include <veriblock/pop/storage/payloads_index.hpp>
#include <veriblock/pop/storage/payloads_provider.hpp>
#include <veriblock/pop/trace.hpp>

namespace altintegration {

namespace internal {

template <typename index_t>
void assertBlockCanBeApplied(index_t& index) {
  VBK_ASSERT_MSG(!index.isRoot(), "cannot apply the root block");

  VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_ACTIVE),
                 "state corruption: tried to apply a block that follows an "
                 "unapplied block %s",
                 index.pprev->toPrettyString());

  VBK_ASSERT_MSG(!index.hasFlags(BLOCK_ACTIVE),
                 "state corruption: tried to apply an already applied block %s",
                 index.toPrettyString());
  // an expensive check; might want to  disable it eventually
  VBK_ASSERT_MSG_DEBUG(
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
  VBK_ASSERT_MSG(!index.isRoot(), "cannot unapply the root block");
  VBK_ASSERT_MSG(!index.finalized, "cannot unapply finalized block");
  VBK_ASSERT_MSG(
      index.hasFlags(BLOCK_ACTIVE),
      "state corruption: tried to unapply an already unapplied block %s",
      index.toPrettyString());
  VBK_ASSERT_MSG(index.pprev->hasFlags(BLOCK_ACTIVE),
                 "state corruption: tried to unapply a block that follows an "
                 "unapplied block %s",
                 index.pprev->toPrettyString());
  // an expensive check; might want to  disable it eventually
  VBK_ASSERT_MSG_DEBUG(
      index.allDescendantsUnapplied(),
      "state corruption: tried to unapply a block before unapplying "
      "its applied descendants %s",
      index.toPrettyString());
}

}  // namespace internal

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
  using command_group_store_t = typename ProtectedTree::command_group_store_t;

  PopStateMachine(ProtectedTree& ed,
                  ProtectingBlockTree& ing,
                  PayloadsIndex& payloadsIndex)
      : ed_(ed),
        ing_(ing),
        commandGroupStore_(ed_.getCommandGroupStore()),
        payloadsIndex_(payloadsIndex) {}

  //! @invariant atomic - applies either all or none of the block's commands
  VBK_CHECK_RETURN bool applyBlock(index_t& index, ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;

    internal::assertBlockCanBeApplied(index);

    if (!index.isValid()) {
      return state.Invalid(
          index_t::block_t::name() + "-marked-invalid",
          format("block {} is marked as invalid and cannot be applied",
                 index.toPrettyString()));
    }

    VBK_ASSERT_MSG(index.isValid(BLOCK_CONNECTED),
                   "attempted to apply an unconnected block %s",
                   index.toPrettyString());

    if (index.hasPayloads()) {
      auto cgroups = commandGroupStore_.getCommands(index, state);
      // TODO:
      VBK_ASSERT_MSG(
          cgroups, "support for payload pre-validation is not implemented yet");

      for (auto cgroup = cgroups->cbegin(); cgroup != cgroups->cend();
           ++cgroup) {
        VBK_LOG_DEBUG("Applying payload %s from block %s",
                      HexStr((*cgroup)->id),
                      index.toShortPrettyString());

        CommandGroup& cg = *(*cgroup);

        if (!cg.execute(state)) {
          VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                        index_t::block_t::name(),
                        index.toPrettyString(),
                        state.toString());

          // unexecute executed command groups in the reverse order
          for (auto r_group = std::reverse_iterator<decltype(cgroup)>(cgroup);
               r_group != cgroups->rend();
               ++r_group) {
            (*r_group)->unExecute();
          }

          ed_.invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);

          return state.Invalid(index_t::block_t::name() + "-bad-command");
        }

      }  // end for
    }

    // if the applied block count equals the size of the chain between the root
    // and index, we have just applied a block on top of the only applied chain,
    // so it is fully valid
    auto appliedChainBlockCount =
        ed_.getRoot().getHeight() +
        (typename block_t::height_t)ed_.appliedBlockCount;

    if (index.pprev->isValid(BLOCK_CAN_BE_APPLIED) &&
        index.getHeight() == appliedChainBlockCount) {
      index.raiseValidity(BLOCK_CAN_BE_APPLIED);
    } else {
      // this block is applied together with the other chain during POP FR
      index.raiseValidity(BLOCK_CAN_BE_APPLIED_MAYBE_WITH_OTHER_CHAIN);
    }

    index.setFlag(BLOCK_ACTIVE);
    ++ed_.appliedBlockCount;

    return true;
  }

  /**
   * Removes all side effects made by this block.
   * @param[in] index block to unapply
   * @invariant atomic
   */
  void unapplyBlock(index_t& index) {
    VBK_TRACE_ZONE_SCOPED;

    internal::assertBlockCanBeUnapplied(index);

    if (index.hasPayloads()) {
      ValidationState state;
      auto cgroups = commandGroupStore_.getCommands(index, state);
      // TODO:
      VBK_ASSERT_MSG(
          cgroups, "support for payload pre-validation is not implemented yet");

      for (const auto& cgroup : reverse_iterate(*cgroups)) {
        VBK_LOG_DEBUG("Unapplying payload %s from block %s",
                      HexStr(cgroup->id),
                      index.toShortPrettyString());
        cgroup->unExecute();
      }
    }

    index.unsetFlag(BLOCK_ACTIVE);
    VBK_ASSERT(ed_.appliedBlockCount > 0);
    --ed_.appliedBlockCount;
  }

  /**
   * Unapply all commands commands from blocks in the range of [from; to)
   * while the predicate returns true. Stop if the predicate returns false.
   * @return the block index on which the predicate returns false or 'to' if the
   * predicate returns true for all blocks
   *
   * @invariant atomic - either unapplies all of the requested blocks or fails
   * on an assert
   */
  VBK_CHECK_RETURN index_t& unapplyWhile(
      index_t& from,
      index_t& to,
      const std::function<bool(index_t& index)>& pred) {
    VBK_TRACE_ZONE_SCOPED;

    if (&from == &to) {
      return to;
    }

    VBK_LOG_DEBUG("Unapply %d blocks from=%s, to=%s",
                  from.getHeight() - to.getHeight(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* current = &from; current != &to; current = current->getPrev()) {
      VBK_ASSERT_MSG(current != nullptr,
                     "reached the genesis or first bootstrap block");
      VBK_ASSERT_MSG(current->getHeight() > to.getHeight(),
                     "[from, to) is not a chain, detected at %s",
                     current->toPrettyString());

      if (!pred(*current)) {
        return *current;
      }

      unapplyBlock(*current);
    }

    return to;
  }

  //!@ overload
  VBK_CHECK_RETURN index_t& unapplyWhile(
      Chain<index_t>& chain, const std::function<bool(index_t& index)>& pred) {
    return unapplyWhile(*chain.tip(), *chain.first(), pred);
  }

  //! unapplies all commands from blocks in the range of [from; to)
  void unapply(index_t& from, index_t& to) {
    VBK_TRACE_ZONE_SCOPED;

    auto pred = [](index_t&) -> bool { return true; };
    auto& firstUnprocessed = unapplyWhile(from, to, pred);
    VBK_ASSERT(&firstUnprocessed == &to);
  }

  //! unapplies all commands in chain
  //! @overload
  void unapply(Chain<index_t>& chain) {
    VBK_ASSERT(!chain.empty());
    unapply(*chain.tip(), *chain.first());
  }

  //! unapply all blocks in the range [slice.tip(); slice.first())
  //! @overload
  void unapply(ChainSlice<index_t>& chain) {
    VBK_ASSERT(!chain.empty());
    unapply(*chain.tip(), *chain.first());
  }

  //! applies all commands from blocks in the range of (from; to].
  //! @invariant atomic: applies either all or none of the requested blocks
  VBK_CHECK_RETURN bool apply(index_t& from,
                              index_t& to,
                              ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;

    if (&from == &to) {
      // already applied this block
      return true;
    }

    if (!to.isValid()) {
      return state.Invalid(
          index_t::block_t::name() + "-marked-invalid",
          format("block {} is marked as invalid and cannot be applied",
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

  //! applies all commands from blocks in chain.
  //! @invariant atomic: applies either all or none of the requested blocks
  //! @overload
  VBK_CHECK_RETURN bool apply(Chain<index_t>& chain, ValidationState& state) {
    return apply(*chain.first(), *chain.tip(), state);
  }

  // effectively unapplies [from; genesis) and applies (genesis; to]
  // assumes and requires that [from; genesis) is applied
  // optimization: avoids applying/unapplying (genesis; fork_point(from, to)]
  // atomic: either changes the state to 'to' or leaves it unchanged
  VBK_CHECK_RETURN bool setState(index_t& from,
                                 index_t& to,
                                 ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;

    if (&from == &to) {
      // already at this state
      return true;
    }

    const auto* forkBlock = getForkBlock(from, to);
    VBK_ASSERT_MSG(forkBlock, "Fork Block must exist!");

    auto& fork = as_mut(*forkBlock);
    unapply(from, fork);
    if (!apply(fork, to, state)) {
      // attempted to switch to an invalid block, rollback
      bool success = apply(fork, from, state);
      VBK_ASSERT_MSG(success,
                     "state corruption: failed to rollback the state: %s",
                     state.toString());

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
  command_group_store_t& commandGroupStore_;
  PayloadsIndex& payloadsIndex_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
