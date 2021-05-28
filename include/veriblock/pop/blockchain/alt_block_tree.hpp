// Copyright (c) 2019-2021 Xenios SEZC
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
#include <veriblock/pop/blockchain/commands/alt_command_group_store.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/fmt.hpp>
#include <veriblock/pop/storage/block_reader.hpp>
#include <veriblock/pop/storage/payloads_index.hpp>
#include <veriblock/pop/storage/payloads_provider.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "alt_chain_params.hpp"
#include "base_block_tree.hpp"
#include "chain.hpp"
#include "pop/fork_resolution.hpp"
#include "pop/vbk_block_tree.hpp"
namespace altintegration {

// defined in alt_block_tree.cpp
extern template struct BlockIndex<AltBlock>;
extern template struct BaseBlockTree<AltBlock>;

// clang-format off
//! @struct AltBlockTree
//!
//! @brief Represents simplified view on Altchain's block tree, maintains VBK tree and
//! BTC tree.
//!
//! Helps Altchain to compare Altchain blocks by POP score in POP Fork Resolution.
//! @invariant If POP score of two chains are equal, it expects Altchain to resolve conflict using their native fork resolution algorithm.
//!
//! AltBlockTree is initialized with Altchain, Veriblock and Bitcoin parameters, as well as with PayloadsStorage implementation.
//! ```cpp
//! // use mainnet for all chains, for instance
//! AltChainParamsMain altp;
//! VbkChainParamsMain vbkp;
//! BtcChainParamsMain btcp;
//! // your implementation of PayloadsStorage
//! PayloadsProviderImpl provider;
//! // your implementation of BlockReader
//! BlockReaderImpl blockProvider
//! AltBlockTree tree(altp, vbkp, btcp, provider, blockProvider);
//! ```
//!
//! After initialization AltBlockTree does not contain any blocks.
//! **Users MUST bootstrap AltBlockTree** - add initial/genesis/root block of Altchain.
//!
//! We will refer to this block as **bootstrap block**.
//! Bootstrap block is a first block in Altchain that can be endorsed, but can not contain POP body (PopData).
//!
//! If `bootstrap()` fails, your block provided in struct AltChainParams::getBootstrapBlock() is invalid.
//!
//! ValidationState will contain detailed info about failure reason, if any.
//!
//!```cpp
//! ValidationState state;
//! bool ret = tree.bootstrap(state);
//! assert(ret && "bootstrap is unsuccessful");
//!```
//!
//! @invariant bootstrap block is immediately finalized - it can not be reorganized.
//!
//! Whenever any new **full block** (block with header, block body and PopData) is accepted,
//! users must first add block header to AltBlockTree, then add PopData to this block.
//!
//!```cpp
//! bool onNewFullBlock(AltBlockTree& tree, Block block) {
//!     ValidationState state;
//!
//!     // first, add block header. if this returns false,
//!     // header can not be connected (invalid).
//!     if(!tree.acceptBlockHeader(block.getHeader(), state)) {
//!         //! block header is invalid. Use state to get info about failure.
//!         return false;
//!     }
//!
//!     // then, attach block body (PopData) to block header.
//!     // if block does not exist, this will fail on assert.
//!     tree.acceptBlock(block.getHash(), block.getPopData());
//!
//!     // ...
//! ```
//!
//! AltBlockTree::acceptBlockHeader() connects block immediately if all previous blocks are connected, or just adds block body to AltBlock when one of previous blocks is not connected.
//!
//! After that, users can check if this block is connected:
//! ```cpp
//!     // ...
//!     // this returns nullptr if block can not be found
//!     auto* blockindex = tree.getBlockIndex(block.getHash());
//!     assert(blockindex && "we added this block to a tree, so it must exist");
//!
//!     auto candidates = tree.getConnectedTipsAfter(*blockindex);
//!     if(candidates.empty()) {
//!       // we have no POP FR candidates
//!       return true; //! block have been added
//!     }
//!
//!     const auto* tip = tree.getBestChain().tip();
//!     for(const auto* candidate : candidates) {
//!         // here, we assume that candidate has all txes downloaded and block is fully available
//!
//!         // compare current tip to a candidate
//!         int result = tree.comparePopScore(tip->getHash(), candidate->getHash());
//! ```
//!
//! @note after AltBlockTree::comparePopScore AltBlockTree always corresponds to a state, as if winner chain have been applied.
//!
//!```cpp
//!         if(result < 0) {
//!             // candidate has better POP score.
//!             // tree already switched to candidate chain.
//!
//!             UpdateTip(candidate->getHash());
//!             // NOTE: update `tip`, otherwise old tip will be passed to first arg,
//!             // and comparePopScore will die on assert
//!             tip = candidate;
//!             return true;
//!         } else if (result == 0) {
//!             // tip POP score == candidate POP score
//!             // tree tip is unchanged
//!             // fallback to chain-native Fork Resolution algorithm
//!
//!             // in BTC, for example, compare blocks by chainwork
//!             if(tip->nChainWork < candidate->nChainWork) {
//!                 //! candidate has better chainwork
//!                 UpdateTip(candidate->getHash());
//!             }
//!             return true;
//!        } else {
//!             // candidate is invalid or has worse POP score
//!             // tree tip is unchanged
//!             return true;
//!         }
//!     } //! end of forloop
//! } //! end of OnNewFullBlock
//! ```
//!
//! @invariant AltBlockTree::comparePopScore always compares current AltBlockTree tip to other block. To avoid confusion, you must specify tip explicitly as first arg. If incorrect tip is passed, function dies on assert.
//!
//! @invariant AltBlockTree::comparePopScore always leaves AltBlockTree switched to winner (by POP Score) chain.
//!
//! @invariant Current active chain of AltBlockTree always corresponds to an empty tree with all applied blocks from first bootstrap block to current tip, i.e. currently applied active chain and this state MUST be always valid.
//!
//! When tip is changed, Altchain MUST change state of AltBlockTree:
//!
//! @note use AltBlockTree::setState() to switch from current best chain to new block. It can very expensive operation if there's large reorg. If altchain is already at this state, this is a no-op.
//!
//! ```cpp
//! void UpdateTip(uint256 bestHash) {
//!   ValidationState state;
//!   //! setState returns true if all blocks on path (tip...bestHash] are valid
//!   bool ret = tree.setState(bestHash, state);
//!   assert(ret && "this block won FR, so it must be valid");
//! }
//! ```
//! @invariant Current tip of your Altchain tree MUST correspond to `tree.getBestChain().tip()`, so calling AltBlockTree::setState() ensures they are in sync.
//!
//! @see PayloadsStorage
//! @see AltChainParams
//! @see VbkChainParams
//! @see BtcChainParams

// clang-format on
struct AltBlockTree final : public BaseBlockTree<AltBlock> {
  using base = BaseBlockTree<AltBlock>;
  using alt_config_t = AltChainParams;
  using vbk_config_t = VbkChainParams;
  using btc_config_t = BtcChainParams;
  using index_t = base::index_t;
  using stored_index_t = base::stored_index_t;
  using endorsement_t = typename index_t::endorsement_t;
  using eid_t = typename endorsement_t::id_t;
  using hash_t = typename AltBlock::hash_t;
  using command_group_store_t = AltCommandGroupStore;

  using PopForkComparator = PopAwareForkResolutionComparator<AltBlock,
                                                             AltChainParams,
                                                             VbkBlockTree,
                                                             AltBlockTree>;

  virtual ~AltBlockTree() = default;

  explicit AltBlockTree(const alt_config_t& alt_config,
                        const vbk_config_t& vbk_config,
                        const btc_config_t& btc_config,
                        PayloadsStorage& payloadsProvider,
                        BlockReader& blockProvider);

  /**
   * Set very first (bootstrap) altchain block with enabled POP.
   *
   * Call this method before any use of AltBlockTree.
   *
   * @param[out] state validation state
   * @return true in success, false if block is invalid.
   * @private
   */
  VBK_CHECK_RETURN bool bootstrap(ValidationState& state);

  /**
   * Validate and add ALT block header to AltBlockTree.
   * @param[in] block ALT block header
   * @param[out] state validation state
   * @return true if block is valid, and added; false otherwise.
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
   * @pre can be executed on blocks in random order. I.e.
   * @pre must not be executed twice on the same block
   * @pre must not be executed on bootstrap block
   * @pre PopData must be statelessly validated before passing it here
   *
   * @throws StateCorruptedException when we detect state corruption and we can
   * not recover.
   *
   * @param[in] block hash of block where to add the block body
   * @param[in] payloads all POP payloads stored in this block
   *
   */
  void acceptBlock(const hash_t& block, const PopData& payloads);
  //! @overload
  void acceptBlock(index_t& index,
                   const PopData& payloads,
                   ValidationState& state);
  //! @overload
  void acceptBlock(index_t& index, const PopData& payloads);

  /**
   * Get all connected tips after given block.
   * @param[in] index input block
   * @return vector of blocks where every block is connected and after index.
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
   */
  VBK_CHECK_RETURN bool loadBlock(const stored_index_t& index,
                                  ValidationState& state) override;

  /**
   * After all blocks loaded, efficiently set current tip.
   * @param[in] hash tip hash
   * @param[out] state validation state
   * @return true on success, false otherwise
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
   * @pre this function can be called only on existing blocks
   * @pre this function can be called only on connected blocks, otherwise
   * it dies on assert
   * @pre the first argument must always be equal to the current tip of
   * AltBlockTree, otherwise  it dies on assert
   * @pre no other blocks but [genesis .. current tip] must be applied
   * @invariant If B loses the fork resolution, only partial validation of B is
   * performed. We can not determine B validity, because we never applied B
   * alone.
   * @invariant If B wins the fork resolution, it is fully validated and
   * AltBlockTree::setState(B,...) will be successful.
   * @invariant If POP score of both chains is equal, we expect altchain to
   * determine best chain and then change state of AltBlockTree via setState.
   * @post if neither chain wins, A stays applied.
   * @post if chain B wins, the tree automatically switches to chain B (it
   * becomes the tip)
   * @post if return value is more or equal to 0, tree state is unchanged.
   *
   * @throws StateCorruptedException when we detect state corruption and we can
   * not recover.
   *
   * @warning Operation can be expensive for long forks.
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
   */
  VBK_CHECK_RETURN bool setState(index_t& to, ValidationState& state) override;
  //! @overload
  using base::setState;

  //! @private
  bool finalizeBlock(index_t& index, ValidationState& state);

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
   * @private
   */
  void removePayloads(const hash_t& hash);

  // clang-format off
  //! @private
  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  //! Accessor for VBK tree
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  //! @private
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  //! Accessor for BTC tree
  const VbkBlockTree::BtcTree& btc() const { return cmp_.getProtectingBlockTree().btc(); }
  //! @private
  const PopForkComparator& getComparator() const { return cmp_; }
  //! Accessor for Network Parameters stored in this tree
  const AltChainParams& getParams() const { return *alt_config_; }
  //! @private
  PayloadsIndex& getPayloadsIndex()  { return payloadsIndex_; }
  //! @private
  const PayloadsIndex& getPayloadsIndex() const { return payloadsIndex_; }
  //! @private
  PayloadsStorage& getPayloadsProvider()  { return payloadsProvider_; }
  //! @private
  const PayloadsStorage& getPayloadsProvider() const { return payloadsProvider_; }
  //! @private
  AltCommandGroupStore& getCommandGroupStore()  { return commandGroupStore_; }
  //! @private
  const AltCommandGroupStore& getCommandGroupStore() const { return commandGroupStore_; }

  // clang-format on

  //! @private
  std::string toPrettyString(size_t level = 0) const;

  //! @private
  //! @invariant the tip must be fully validated
  void overrideTip(index_t& to) override;

  friend struct MemPoolBlockTree;

  //! @private
  using base::removeLeaf;

 private:
  const alt_config_t* alt_config_;
  PopForkComparator cmp_;
  PayloadsIndex payloadsIndex_;
  PayloadsStorage& payloadsProvider_;
  BlockReader& blockProvider_;
  command_group_store_t commandGroupStore_;

  //! @private
  void determineBestChain(index_t& candidate, ValidationState& state) override;

  //! @private
  void setPayloads(index_t& index, const PopData& payloads);

  //! @private
  bool finalizeBlockImpl(index_t& index,
                         int32_t preserveBlocksBehindFinal,
                         ValidationState& state) override;

  /**
   * Connect the block to the tree, doing stateful validation(incomplete at this
   * moment)
   * @return true if the block is statefully valid
   * @private
   */
  bool connectBlock(index_t& index, ValidationState& state);

  //! @private
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
  for (const auto* e : i.getEndorsedBy()) {
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
