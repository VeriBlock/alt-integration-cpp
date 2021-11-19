// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <unordered_map>
#include <utility>

#include "veriblock/pop/blockchain/blocktree.hpp"
#include "veriblock/pop/blockchain/commands/vbk_command_group_store.hpp"
#include "veriblock/pop/blockchain/pop/fork_resolution.hpp"
#include "veriblock/pop/blockchain/pop/pop_state_machine.hpp"
#include "veriblock/pop/blockchain/vbk_block_addon.hpp"
#include "veriblock/pop/blockchain/vbk_chain_params.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/finalizer.hpp"
#include "veriblock/pop/storage/block_reader.hpp"
#include "veriblock/pop/storage/payloads_index.hpp"

namespace altintegration {

template <>
inline void BaseBlockTree<BtcBlock>::decreaseAppliedBlockCount(size_t) {
  // do nothing
  // BTC tree is not protected
}

template <>
inline void BaseBlockTree<BtcBlock>::increaseAppliedBlockCount(size_t) {
  // do nothing
  // BTC tree is not protected
}

// defined in vbk_block_tree.cpp
extern template struct BlockIndex<BtcBlock>;
extern template struct BlockTree<BtcBlock, BtcChainParams>;
extern template struct BaseBlockTree<BtcBlock>;
extern template struct BlockIndex<VbkBlock>;
extern template struct BlockTree<VbkBlock, VbkChainParams>;
extern template struct BaseBlockTree<VbkBlock>;

//! Bitcoin tree
using BtcBlockTree = BlockTree<BtcBlock, BtcChainParams>;

/**
 * @class VbkBlockTree
 *
 * Veriblock block tree.
 *
 * It is strongly advised to not modify the Veriblock tree directly, as it is
 * intended to be fully managed by AltBlockTree
 *
 * @invariant Adding an invalid payload to block X cannot invalidate any block
 * but X. As a consequence, the tree allows duplicates and AltBlockTree must
 * check that an altchain fork does not contain VTB duplicates.
 * @invariant addPayloads fully validates each payload and refuses to add
 * invalid payloads.
 * @invariant removePayloads and unsafelyRemovePayload cannot invalidate any
 * block as long as they are called by AltBlockTree, and not by the user.
 * @invariant all payloads and blocks are valid as a consequence of the above.
 *
 * Definition: a validation hole is incorrect use of removePayloads and
 * unsafelyRemovePayload by AltBlockTree that results in invalid Veriblock
 * tree blocks or corrupted state.
 *
 * Notes regarding the validation hole:
 * At this moment, the validation hole can be considered plugged as long as the
 * user never modifies the VBK tree directly, never uses PopStateMachine and
 * only calls AltBlockTree::setState() and AltBlockTree::activateBestChain() as
 * currently implemented. However, a trivial code modification can
 * unintentionally re-introduce the issue.
 *
 * To plug the validation hole, we must ensure that any unvalidated block is
 * unapplied before all of the blocks that were applied at the time of the
 * application.
 *
 * Suppose, altchain blocks were applied in the following order:
 *        V1, V2, V3, U4, U5, U6, V7, V8, U9
 *        (V* are fully valid blocks, U* are unvalidated blocks)
 *
 * In this example, AltBlockTree must ensure that the blocks are unapplied in
 * the following sequence:
 *      * U9 is unapplied first
 *      * V8 and V7 in any order; both V8, V7 and V7, V8 are safe
 *      * U6, U5, U4 in this specific order
 *      * V3-V1 in any order
 *
 * This looks like a feasible safeguard:
 *      * Prevent the user from modifying the VBK tree
 *      * Add a list stack that stores all blocks in the order they were
 *        applied. On application, blocks are appended to the list. On
 *        unapplication, blocks are deleted from the list. A block can be
 *        unapplied only if it is contained in the list slice beween the tail
 *        and the last unvalidated block.
 *
 * In this example, initially, the slice is [U9]. After we unapply U9, the slice
 * becomes [U6, V7, V8]. If U6 is unapplied, the slice becomes [U5, V7, V8].
 */
struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BtcBlockTree;
  using index_t = VbkTree::index_t;
  using stored_index_t = VbkTree::stored_index_t;
  using payloads_t = typename index_t::payloads_t;
  using pid_t = typename payloads_t::id_t;
  using endorsement_t = typename index_t::endorsement_t;
  using command_group_store_t = VbkCommandGroupStore;
  using PopForkComparator = PopAwareForkResolutionComparator<VbkBlock,
                                                             VbkChainParams,
                                                             BtcTree,
                                                             VbkBlockTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp,
               const BtcChainParams& btcp,
               PayloadsStorage& payloadsProvider,
               BlockReader& blockProvider,
               PayloadsIndex& payloadsIndex);

  //! efficiently connect `index` to current tree as a leaf, loaded from disk
  //! - recovers all pointers (pprev, pnext, endorsedBy)
  //! - recalculates chainWork
  //! - does validation of endorsements
  //! - recovers tips array
  //! @invariant NOT atomic.
  bool loadBlockForward(const stored_index_t& index,
                        ValidationState& state) override;

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  PopForkComparator& getComparator() { return cmp_; }
  const PopForkComparator& getComparator() const { return cmp_; }
  PayloadsIndex& getPayloadsIndex() { return payloadsIndex_; }
  //! @private
  VbkCommandGroupStore& getCommandGroupStore() { return commandGroupStore_; }
  //! @private
  const VbkCommandGroupStore& getCommandGroupStore() const {
    return commandGroupStore_;
  }

  bool loadTip(const hash_t& hash, ValidationState& state) override;

  /**
   * Attempts to add payloads to the block and perform full validation.
   * If successful, it is possible to setState() to the block after the
   * addPayloads call. If unsuccessful, it leaves the state unchanged.
   * @invariant atomic: adds either all or none of the payloads
   */
  bool addPayloads(const VbkBlock::hash_t& hash,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  void removePayloads(const hash_t& hash, const std::vector<pid_t>& pids);
  void removePayloads(const block_t& block, const std::vector<pid_t>& pids);
  void removePayloads(index_t& index, const std::vector<pid_t>& pids);

  /**
   * If we add payloads to the VBK tree in the following order: A1, B2, A3.
   *
   * Ending up with the tree looking like this:
   * A(1,3)-o-o-o-B(2)
   *
   * It is only safe to use this function to remove them in the opposite order:
   * A3, B2, A1; or A3, B2.
   *
   * It is unsafe to use this function to remove them in any other order eg:
   * B2, A3, A1; or just B2.
   */
  void unsafelyRemovePayload(const Blob<24>& hash, const pid_t& pid);
  void unsafelyRemovePayload(const block_t& block, const pid_t& pid);
  void unsafelyRemovePayload(index_t& index,
                             const pid_t& pid,
                             bool shouldDetermineBestChain = true);

  std::string toPrettyString(size_t level = 0) const;

  void overrideTip(index_t& to) override;

  bool setState(index_t& to, ValidationState& state) override;

  bool setState(const hash_t& block, ValidationState& state) override;

  void removeSubtree(index_t& toRemove) override;

 private:
  bool loadBlockInner(const stored_index_t& index, ValidationState& state);

  void finalizeBlockImpl(index_t& index,
                         int32_t preserveBlocksBehindFinal) override;

  bool validateBTCContext(const payloads_t& vtb, ValidationState& state);
  /**
   * Add, apply and validate a payload to a block that's currently applied
   *
   * Will add duplicates.
   * The containing block must be applied and must be valid as a consequence
   * @invariant atomic: leaves the state unchanged on failure
   * @return: true/false on success/failure
   */
  bool addPayloadToAppliedBlock(index_t& index,
                                const payloads_t& payload,
                                ValidationState& state);

  void determineBestChain(index_t& candidate, ValidationState& state) override;

  PopForkComparator cmp_;
  PayloadsStorage& payloadsProvider_;
  PayloadsIndex& payloadsIndex_;
  command_group_store_t commandGroupStore_;
};

//! @private
template <>
void assertBlockCanBeRemoved(const BlockIndex<BtcBlock>& index);
//! @private
template <>
void assertBlockCanBeRemoved(const BlockIndex<VbkBlock>& index);

//! @private
template <>
void assertBlockSanity(const VbkBlock& block);

//! @private
template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    VbkBlockTree& tree,
    const std::vector<VTB>& pop,
    const std::vector<uint8_t>& containinghash);

//! @private
template <>
void payloadToCommands(VbkBlockTree& tree,
                       const VTB& pop,
                       const std::vector<uint8_t>& containingHash,
                       std::vector<CommandPtr>& cmds);

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const BlockIndex<VbkBlock>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "chainWork", i.chainWork.toHex());

  std::vector<uint256> endorsements;
  for (const auto& e : i.getContainingEndorsements()) {
    endorsements.push_back(e.first);
  }
  json::putArrayKV(obj, "containingEndorsements", endorsements);

  std::vector<uint256> endorsedBy;
  for (const auto* e : i.getEndorsedBy()) {
    endorsedBy.push_back(e->id);
  }
  json::putArrayKV(obj, "endorsedBy", endorsedBy);
  json::putIntKV(obj, "height", i.getHeight());
  json::putKV(obj, "header", ToJSON<JsonValue>(i.getHeader()));
  json::putIntKV(obj, "status", i.getStatus());
  json::putIntKV(obj, "altrefs", i.refCount());

  auto stored = json::makeEmptyObject<JsonValue>();
  json::putArrayKV(stored, "vtbids", i.getPayloadIds<VTB>());
  json::putKV(obj, "stored", stored);

  auto bopEndorsements = json::makeEmptyArray<JsonValue>();
  for (const auto* e : i.getBlockOfProofEndorsement()) {
    if (e == nullptr) {
      continue;
    }
    json::arrayPushBack(bopEndorsements, ToJSON<JsonValue>(e->getId()));
  }
  json::putKV(obj, "blockOfProofEndorsements", bopEndorsements);

  return obj;
}

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const BlockIndex<BtcBlock>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "chainWork", i.chainWork.toHex());
  json::putIntKV(obj, "height", i.getHeight());
  json::putKV(obj, "header", ToJSON<JsonValue>(i.getHeader()));
  json::putIntKV(obj, "status", i.getStatus());
  json::putArrayKV(obj, "vbkrefs", i.getRefs());

  auto bopEndorsements = json::makeEmptyArray<JsonValue>();
  for (const auto* e : i.getBlockOfProofEndorsement()) {
    if (e == nullptr) {
      continue;
    }
    json::arrayPushBack(bopEndorsements, ToJSON<JsonValue>(e->getId()));
  }
  json::putKV(obj, "blockOfProofEndorsements", bopEndorsements);

  return obj;
}

//! HACK: getBlockIndex accepts either hash_t or prev_block_hash_t
//! then, depending on what it received, it should do trim LE on full hash to
//! receive short hash, which is stored inside a map. In this weird case, when
//! Block=VbkBlock, we may call `getBlockIndex(block->previousBlock)`, it is a
//! call `getBlockIndex(Blob<12>). But when `getBlockIndex` accepts it, it does
//! an implicit cast to full hash (hash_t), adding zeroes in the end. Then,
//! .trimLE returns 12 zeroes.
//!
//! This hack allows us to inject explicit conversion hash_t (Blob<24>) ->
//! prev_block_hash_t (Blob<12>).
//! @private
template <>
template <>
inline BaseBlockTree<VbkBlock>::prev_block_hash_t
BaseBlockTree<VbkBlock>::makePrevHash<BaseBlockTree<VbkBlock>::hash_t>(
    const hash_t& h) const {
  // do an explicit cast from hash_t -> prev_block_hash_t
  return h.template trimLE<prev_block_hash_t::size()>();
}

//! @private
inline void PrintTo(const VbkBlockTree& tree, std::ostream* os) {
  *os << tree.toPrettyString();
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
