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

namespace {
void setValidity(const altintegration::CommandGroup& cm,
                 bool valid,
                 altintegration::PayloadsStorage& storage) {
  if (cm.getPayloadsTypeName() == altintegration::ATV::name()) {
    storage.setValidity<altintegration::ATV>(altintegration::ATV::id_t(cm.id),
                                             valid);
  }

  if (cm.getPayloadsTypeName() == altintegration::VTB::name()) {
    storage.setValidity<altintegration::VTB>(altintegration::VTB::id_t(cm.id),
                                             valid);
  }

  if (cm.getPayloadsTypeName() == altintegration::VbkBlock::name()) {
    storage.setValidity<altintegration::VbkBlock>(
        altintegration::VbkBlock::id_t(cm.id), valid);
  }
}

}  // namespace

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
                  height_t startHeight = 0)
      : ed_(ed), ing_(ing), storage_(storage), startHeight_(startHeight) {}

  // atomic: applies either all or none of the block's commands
  bool applyBlock(index_t& index, ValidationState& state) {
    std::vector<CommandPtr> executed;
    auto cgs = storage_.loadCommands<ProtectedTree>(index, ed_);
    // even if the block is marked as invalid, we still try to apply it
    for (const auto& cg : cgs) {
      VBK_LOG_DEBUG("Applying payload %s from block %s",
                    HexStr(cg.id),
                    index.toShortPrettyString());

      for (auto& cmd : cg.commands) {
        if (!cmd->Execute(state)) {
          // invalidate command group
          ::setValidity(cg, false, storage_);
          VBK_LOG_ERROR("Invalid %s command in block %s: %s",
                        index_t::block_t::name(),
                        index.toPrettyString(),
                        state.toString());

          // unexecute executed commands in reverse order
          for (auto& c : reverse_iterate(executed)) {
            c->UnExecute();
          }

          // if the block is marked as valid, invalidate its subtree
          if (index.isValid()) {
            ed_.invalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
          }

          return state.Invalid(index_t::block_t::name() + "-bad-command");
        }  // end if

        // command is valid
        executed.push_back(cmd);
      }  // end for

      // re-validate command group
      ::setValidity(cg, true, storage_);
    }  // end for

    // we successfully applied the block
    // if the block is marked as invalid, revalidate its subtree
    if (!index.isValid()) {
      ed_.revalidateSubtree(index, BLOCK_FAILED_POP, /*do fr=*/false);
    }
    return true;
  }

  // atomic: applies either all of the block's commands or fails on an assert
  void unapplyBlock(const index_t& index) {
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

  // unapplies all commands commands from blocks in the range of [from; to)
  // atomic: either applies all of the requested blocks or fails on an assert
  void unapply(index_t& from, index_t& to) {
    if (&from == &to) {
      return;
    }

    VBK_ASSERT(from.height > to.height);
    // exclude 'to' by adding 1
    Chain<index_t> chain(to.height + 1, &from);
    VBK_ASSERT(chain.first());
    VBK_ASSERT(chain.first()->pprev == &to);

    VBK_LOG_DEBUG("Unapply %d blocks from=%s, to=%s",
                  chain.blocksCount(),
                  from.toPrettyString(),
                  to.toPrettyString());

    for (auto* current : reverse_iterate(chain)) {
      if (current->payloadsIdsEmpty()) {
        continue;
      }

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

    VBK_ASSERT(from.height < to.height);
    // exclude 'from' by adding 1
    Chain<index_t> chain(from.height + 1, &to);
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
  // optimization: avoids applying/unapplying (genesis; last_common_block(from,
  // to)] atomic: either changes the state to 'to' or leaves it unchanged
  bool setState(index_t& from, index_t& to, ValidationState& state) {
    if (VBK_UNLIKELY(IsShutdownRequested())) {
      return true;
    }

    if (from == to) {
      // already at this state
      return true;
    }

    // is 'to' a successor?
    if (to.getAncestor(from.height) == &from) {
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
  ProtectedTree& ed_;
  ProtectingBlockTree& ing_;
  PayloadsStorage& storage_;
  height_t startHeight_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
