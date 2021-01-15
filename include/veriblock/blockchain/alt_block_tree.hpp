// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "alt_block_tree_util.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/base_block_tree.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/pop_payouts.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/storage/payloads_index.hpp"
#include "veriblock/storage/payloads_provider.hpp"
#include "veriblock/validation_state.hpp"
namespace altintegration {

// defined in alt_block_tree.cpp
extern template struct BlockIndex<AltBlock>;
extern template struct BaseBlockTree<AltBlock>;

// clang-format off
/**
 * @struct AltBlockTree
 * @brief Represents simplified view on Altchain's block tree, maintains VBK tree and BTC tree.
 * @copydoc altblocktree
 *
 * @ingroup api
 */
// clang-format on
struct AltBlockTree : public BaseBlockTree<AltBlock> {
  using base = BaseBlockTree<AltBlock>;
  using alt_config_t = AltChainParams;
  using vbk_config_t = VbkChainParams;
  using btc_config_t = BtcChainParams;
  using index_t = BlockIndex<AltBlock>;
  using endorsement_t = typename index_t::endorsement_t;
  using eid_t = typename endorsement_t::id_t;
  using hash_t = typename AltBlock::hash_t;

  using PopForkComparator = PopAwareForkResolutionComparator<AltBlock,
                                                             AltChainParams,
                                                             VbkBlockTree,
                                                             AltBlockTree>;

  virtual ~AltBlockTree() = default;

  explicit AltBlockTree(const alt_config_t& alt_config,
                        const vbk_config_t& vbk_config,
                        const btc_config_t& btc_config,
                        PayloadsProvider& storagePayloads);

  /**
   * Set very first (bootstrap) altchain block with enabled POP.
   *
   * Call this method before any use of AltBlockTree.
   *
   * @param[out] state validation state
   * @return true in success, false if block is invalid.
   * @ingroup api
   */
  VBK_CHECK_RETURN bool bootstrap(ValidationState& state);

  /**
   * Validate and add ALT block header to AltBlockTree.
   * @param[in] block ALT block header
   * @param[out] state validation state
   * @return true if block is valid, and added; false otherwise.
   * @ingroup api
   */
  VBK_CHECK_RETURN bool acceptBlockHeader(const AltBlock& block,
                                          ValidationState& state);

  /**
   * Attach "block body" - PopData to block header, which already exists in
   * AltBlockTree.
   *
   * When block is connected (meaning that all of its ancestors are also
   * connected and have block body), it emits new signal onBlockConnected.
   *
   * @invariant can be executed on blocks in random order
   * @invariant must not be executed twice on the same block
   * @invariant must not be executed on bootstrap block
   * @invariant PopData must be statelessly validated before passing it here
   *
   * @throws StateCorruptedException when we detect state corruption and we can
   * not recover.
   *
   * @param[in] block hash of block where to add the block body
   * @param[in] payloads all POP payloads stored in this block
   *
   * @ingroup api
   */
  void acceptBlock(const hash_t& block, const PopData& payloads);
  //! @overload
  void acceptBlock(index_t& index, const PopData& payloads);

  /**
   * Get all connected tips after given block.
   * @param[in] index input block
   * @return vector of blocks where every block is connected and after index.
   * @ingroup api
   */
  std::vector<const index_t*> getConnectedTipsAfter(const index_t& index) const;

  //! a block has been handed over to the underlying tree and flagged as invalid
  signals::Signal<void(index_t& index, ValidationState&)>
      onInvalidBlockConnected;

  //! a block has been successfully handed over to the underlying tree
  signals::Signal<void(index_t& index)> onBlockConnected;

  //! chain reorg signal - the tip is being changed
  signals::Signal<void(const index_t& index)> onBeforeOverrideTip;

  /**
   * Add a block body to the block header. Can be done once per each block.
   * Blocks with added payloads must form a tree.
   * @param[in] block hash of ALT block where block body is added.
   * @param[in] popData POP block body
   * @param[out] state validation state
   * @return true if PopData does not contain duplicates (searched across active
   * chain). However, it is far from certain that it is completely valid.
   * @ingroup api
   */
  bool addPayloads(const hash_t& block,
                   const PopData& popData,
                   ValidationState& state);
  //! @overload
  bool addPayloads(index_t& index, PopData& payloads, ValidationState& state);

  /**
   * Efficiently connect block loaded from disk.
   *
   * It recovers all pointers (pprev, pnext, endorsedBy,
   * blockOfProofEndorsements), validates block and endorsements, recovers
   * validity index, recovers tips array.
   * @param[in] index block
   * @param[out] state validation state
   * @return true if block is valid
   * @invariant NOT atomic. If loadBlock failed, AltBlockTree state is undefined
   * and can not be used. Tip: ask user to run with '-reindex'.
   * @ingroup api
   */
  VBK_CHECK_RETURN bool loadBlock(const index_t& index,
                                  ValidationState& state) override;

  /**
   * After all blocks loaded, efficiently set current tip.
   * @param[in] hash tip hash
   * @param[out] state validation state
   * @return true on success, false otherwise
   * @ingroup api
   */
  VBK_CHECK_RETURN bool loadTip(const hash_t& hash,
                                ValidationState& state) override;

  /**
   * Efficiently compares current tip (A) and any other block (B).
   *
   * @param[in] A hash of current tip in AltBlockTree. Fails on assert if
   * current tip != A.
   * @param[in] B block. Current tip will be compared against this block. Must
   * exist on chain and be connected(have BLOCK_HAS_PAYLOADS and
   * BLOCK_CONNECTED), but does not have to be fully validated(necessary
   * validation will be performed during the fork resolution).
   * @warning POP Fork Resolution is NOT transitive, it can not be used to
   * search for an "absolute" best chain. If A is better than B, and B is better
   * than C, then A may NOT be better than C. It is expected that caller will
   * execute this comparator once for every tip candidate to avoid cycles
   * (A->B->C->A). But this is no big deal, as sooner or later peers with
   * different chains will converge to the same chain.
   * @return
   * Returns positive if chain A is better.
   * Returns negative if chain B is better.
   * Returns 0 if blocks are equal in terms of POP. Users should fallback to
   * chain-native Fork Resolution algorithm.
   * @invariant this function can be called only on existing blocks
   * @invariant this function can be called only on connected blocks, otherwise
   * it dies on assert
   * @invariant the first argument must always be equal to the current tip of
   * AltBlockTree, otherwise  it dies on assert
   * @invariant no other blocks but [genesis .. current tip] must be applied
   * @invariant If B loses the fork resolution, only partial validation of B is
   * performed.
   * @invariant If B wins the fork resolution, it is fully validated and
   * AltBlockTree::setState(B,...) will be successful.
   * @invariant if neither chain wins, A stays applied.
   * @invariant if chain B wins, the tree automatically switches to chain B (it
   * becomes the tip)
   *
   * @throws StateCorruptedException when we detect state corruption and we can
   * not recover.
   *
   * @warning Operation can be expensive for long forks.
   * @ingroup api
   */
  VBK_CHECK_RETURN int comparePopScore(const AltBlock::hash_t& A,
                                       const AltBlock::hash_t& B);

  /**
   * Switch AltBlockTree from the current tip to different block, while doing
   * all validations of intermediate blocks.
   *
   * @param[in] to tree will be switched to this block
   * @param[out] state
   * @return `false` if intermediate or target block is invalid. In this case
   * tree will rollback into original state. `true` if state change is
   * successful.
   * @invariant atomic - either switches to new state, or does nothing.
   * @invariant the function switches state from exactly [genesis .. current
   * tip] chain being applied(and no other) to [genesis .. to] chain being
   * applied
   * @invariant both the current and new chains are fully valid; full validation
   * of [genesis .. to] is a side effect
   * @warning Expensive operation when we need to do long reorgs.
   * @throws StateCorruptedException when we detect state corruption and we can
   * not recover.
   * @ingroup api
   */
  VBK_CHECK_RETURN bool setState(index_t& to, ValidationState& state) override;
  //! @overload
  using base::setState;

  /**
   * Removes given block and all blocks after it.
   * @param[in] toRemove block to be removed.
   * @warning fails on assert if block can not be found in this tree.
   */
  using base::removeSubtree;
  //! @overload
  void removeSubtree(index_t& toRemove) override;

  /**
   * Removes all payloads from a block
   * @param[in] hash
   * @ingroup api
   */
  void removePayloads(const hash_t& hash);

  // use this method for stateful validation of pop data. invalid pop data will
  // be removed from `pop`
  //! @private
  void filterInvalidPayloads(PopData& pop);

  // clang-format off
  //! @private
  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  //! Accessor for VBK tree
  //! @ingroup api
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  //! @private
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  //! Accessor for BTC tree
  //! @ingroup api
  const VbkBlockTree::BtcTree& btc() const { return cmp_.getProtectingBlockTree().btc(); }
  //! @private
  const PopForkComparator& getComparator() const { return cmp_; }
  //! Accessor for Network Parameters stored in this tree
  //! @ingroup api
  const AltChainParams& getParams() const { return *alt_config_; }
  //! @private
  PayloadsIndex& getPayloadsIndex()  { return payloadsIndex_; }
  //! @private
  const PayloadsIndex& getPayloadsIndex() const { return payloadsIndex_; }
  //! @private
  PayloadsProvider& getPayloadsProvider()  { return payloadsProvider_; }
  //! @private
  const PayloadsProvider& getPayloadsProvider() const { return payloadsProvider_; }
  // clang-format on

  //! @private
  std::string toPrettyString(size_t level = 0) const;

  //! @private
  //! @invariant the tip must be fully validated
  void overrideTip(index_t& to) override;

  //! @private
  using base::removeLeaf;

 protected:
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;
  PayloadsIndex payloadsIndex_;
  PayloadsProvider& payloadsProvider_;

  void determineBestChain(index_t& candidate, ValidationState& state) override;

  void setPayloads(index_t& index, const PopData& payloads);

  /**
   * Connect the block to the tree, doing stateful validation(incomplete at this
   * moment)
   * @return true if the block is statefully valid
   */
  bool connectBlock(index_t& index, ValidationState& state);

  void setTipContinueOnInvalid(index_t& to);

  void removeAllPayloads(index_t& index);
};

//! @private
template <typename JsonValue>
JsonValue ToJSON(const BlockIndex<AltBlock>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
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
  json::putIntKV(obj, "status", i.getStatus());

  auto stored = json::makeEmptyObject<JsonValue>();
  json::putArrayKV(stored, "vbkblocks", i.getPayloadIds<VbkBlock>());
  json::putArrayKV(stored, "vtbs", i.getPayloadIds<VTB>());
  json::putArrayKV(stored, "atvs", i.getPayloadIds<ATV>());

  json::putKV(obj, "stored", stored);

  return obj;
}

//! @private
inline void PrintTo(const AltBlockTree& tree, std::ostream* os) {
  *os << tree.toPrettyString();
}

}  // namespace altintegration

#endif  // !
