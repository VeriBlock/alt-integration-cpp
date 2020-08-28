// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/blockchain/vbk_block_addon.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/storage/payloads_index.hpp>

namespace altintegration {

// defined in vbk_block_tree.cpp
extern template struct BlockIndex<BtcBlock>;
extern template struct BlockTree<BtcBlock, BtcChainParams>;
extern template struct BaseBlockTree<BtcBlock>;
extern template struct BlockIndex<VbkBlock>;
extern template struct BlockTree<VbkBlock, VbkChainParams>;
extern template struct BaseBlockTree<VbkBlock>;

/**
 * @class VbkBlockTree
 */
struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using index_t = VbkTree::index_t;
  using payloads_t = typename index_t::payloads_t;
  using pid_t = typename payloads_t::id_t;
  using endorsement_t = typename index_t::endorsement_t;
  using PopForkComparator = PopAwareForkResolutionComparator<VbkBlock,
                                                             VbkChainParams,
                                                             BtcTree,
                                                             VbkBlockTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp,
               const BtcChainParams& btcp,
               PayloadsProvider& storagePayloads,
               PayloadsIndex& payloadsIndex);

  //! efficiently connect `index` to current tree, loaded from disk
  //! - recovers all pointers (pprev, pnext, endorsedBy)
  //! - recalculates chainWork
  //! - does validation of endorsements
  //! - recovers tips array
  //! @invariant NOT atomic.
  bool loadBlock(const index_t& index, ValidationState& state) override;

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  PopForkComparator& getComparator() { return cmp_; }
  const PopForkComparator& getComparator() const { return cmp_; }
  PayloadsIndex& getPayloadsIndex() { return payloadsIndex_; }

  bool areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2);

  bool areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2);

  /**
   * Compare two vtbs that are weakly equivalent.
   * @param[in] first VTB to compare
   * @param[in] second VTB to compare
   * @return:
   * Return positive if vtb1 is better
   * Return negative if vtb2 is better
   * Return 0 if they are strongly equal
   */
  int weaklyCompare(const VTB& vtb1, const VTB& vtb2);

  bool loadTip(const hash_t& hash, ValidationState& state) override;

  /**
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

  using base::setState;
  bool setState(index_t& to, ValidationState& state) override;

  void removeSubtree(index_t& toRemove) override;

 private:
  bool validateBTCContext(const payloads_t& vtb, ValidationState& state);
  /**
   * Add, apply and validate a payload to a block that's currently applied
   *
   * Will add duplicates.
   * The containing block must be applied
   * @invariant atomic: leaves the state unchanged on failure
   * @return: true/false on success/failure
   */
  bool addPayloadToAppliedBlock(index_t& index,
                                const payloads_t& payload,
                                ValidationState& state);

  void determineBestChain(index_t& candidate, ValidationState& state) override;

  PopForkComparator cmp_;
  PayloadsProvider& payloadsProvider_;
  PayloadsIndex& payloadsIndex_;
};

template <>
void assertBlockCanBeRemoved(const BlockIndex<BtcBlock>& index);
template <>
void assertBlockCanBeRemoved(const BlockIndex<VbkBlock>& index);

template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    VbkBlockTree& tree,
    const std::vector<VTB>& pop,
    const std::vector<uint8_t>& containinghash);

template <>
void payloadToCommands(VbkBlockTree& tree,
                       const VTB& pop,
                       const std::vector<uint8_t>& containingHash,
                       std::vector<CommandPtr>& cmds);

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
  for (const auto* e : i.endorsedBy) {
    endorsedBy.push_back(e->id);
  }
  json::putArrayKV(obj, "endorsedBy", endorsedBy);
  json::putIntKV(obj, "height", i.getHeight());
  json::putKV(obj, "header", ToJSON<JsonValue>(i.getHeader()));
  json::putIntKV(obj, "status", i.getStatus());
  json::putIntKV(obj, "ref", i.refCount());

  auto stored = json::makeEmptyObject<JsonValue>();
  json::putArrayKV(stored, "vtbids", i.getPayloadIds<VTB>());

  json::putKV(obj, "stored", stored);

  return obj;
}

template <typename JsonValue>
JsonValue ToJSON(const BlockIndex<BtcBlock>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "chainWork", i.chainWork.toHex());
  json::putIntKV(obj, "height", i.getHeight());
  json::putKV(obj, "header", ToJSON<JsonValue>(i.getHeader()));
  json::putIntKV(obj, "status", i.getStatus());
  json::putIntKV(obj, "ref", i.refCount());

  return obj;
}

// HACK: getBlockIndex accepts either hash_t or prev_block_hash_t
// then, depending on what it received, it should do trim LE on full hash to
// receive short hash, which is stored inside a map. In this weird case, when
// Block=VbkBlock, we may call `getBlockIndex(block->previousBlock)`, it is a
// call `getBlockIndex(Blob<12>). But when `getBlockIndex` accepts it, it does
// an implicit cast to full hash (hash_t), adding zeroes in the end. Then,
// .trimLE returns 12 zeroes.
//
// This hack allows us to inject explicit conversion hash_t (Blob<24>) ->
// prev_block_hash_t (Blob<12>).
template <>
template <>
inline BaseBlockTree<VbkBlock>::prev_block_hash_t
BaseBlockTree<VbkBlock>::makePrevHash<BaseBlockTree<VbkBlock>::hash_t>(
    const hash_t& h) const {
  // do an explicit cast from hash_t -> prev_block_hash_t
  return h.template trimLE<prev_block_hash_t::size()>();
}

inline void PrintTo(const VbkBlockTree& tree, std::ostream* os) {
  *os << tree.toPrettyString();
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
