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
#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/base_block_tree.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

extern template struct BlockIndex<AltBlock>;

/**
 * @class AltTree
 */
struct AltTree : public BaseBlockTree<AltBlock> {
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
                                                             AltTree>;

  virtual ~AltTree() = default;

  AltTree(const alt_config_t& alt_config,
          const vbk_config_t& vbk_config,
          const btc_config_t& btc_config,
          PayloadsStorage& storagePayloads);

  /**
   * Set very first (bootstrap) altchain block with enabled POP.
   *
   * Call this method before any use of AltTree.
   *
   * @param[out] state validation state
   * @return true in success, false if block is invalid.
   */
  bool bootstrap(ValidationState& state);

  /**
   * Validate and add ALT block header to AltTree.
   * @param[in] block ALT block header
   * @param[out] state validation state
   * @return true if block is valid, and added; false otherwise.
   */
  bool acceptBlockHeader(const AltBlock& block, ValidationState& state);

  /**
   * Add block body to block header. Can be done once per each block, in any
   * order.
   * @param[in] block hash of ALT block where block body is added.
   * @param[in] popData POP block body
   * @param[out] state validation state
   * @return true if PopData does not contain duplicates (searched across active
   * chain). However, it is far from certain that it is completely valid.
   */
  bool addPayloads(const hash_t& block,
                   const PopData& popData,
                   ValidationState& state);

  /**
   * Efficiently connect block loaded from disk.
   *
   * It recovers all pointers (pprev, pnext, endorsedBy,
   * blockOfProofEndorsements), validates block and endorsements, recovers
   * validity index, recovers tips array.
   * @param[in] index block
   * @param[out] state validation state
   * @return true if block is valid
   * @invariant NOT atomic. If loadBlock failed, AltTree state is undefined and
   * can not be used. Tip: ask user to run with '-reindex'.
   */
  bool loadBlock(const index_t& index, ValidationState& state) override;

  /**
   * After all blocks loaded, efficiently set current tip.
   * @param[in] hash tip hash
   * @param[out] state validation state
   * @return true on success, false otherwise
   */
  bool loadTip(const hash_t& hash, ValidationState& state) override;

  /**
   * Efficiently compares current tip (A) and any other block (B).
   *
   * @param[in] A hash of current tip in AltTree. Fails on assert if current
   * tip != A.
   * @param[in] B current tip will be compared against this block. Must
   * exist on chain and have BLOCK_HAS_PAYLOADS.
   * @warning POP Fork Resolution is NOT transitive, it can not be used to
   * search for an "absolute" best chain. If A is better than B, and B is better
   * than C, then A may NOT be better than C. It is expected that caller will
   * execute this comparator once for every tip candidate to avoid cycles
   * (A->B->C->A). But this is no big deal, as sooner or later peers with
   * different chains will eventually converge to the same chain.
   * @return
   * Returns positive if chain A is better.
   * Returns negative if chain B is better.
   * Returns 0 if blocks are equal in terms of POP. Users should fallback to
   * chain-native Fork Resolution algorithm.
   */
  int comparePopScore(const AltBlock::hash_t& A, const AltBlock::hash_t& B);

  /**
   * Calculate payouts for the altchain tip.
   * @param[in] hash of altchain tip.
   * @return map with reward recipient as a key and reward amount as a value
   */
  std::map<std::vector<uint8_t>, int64_t> getPopPayout(const hash_t& tip);

  // removes all payloads from a block
  void removePayloads(const hash_t& hash);

  // use this method for stateful validation of pop data. invalid pop data will
  // be removed from `pop`
  void filterInvalidPayloads(PopData& pop);

  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const VbkBlockTree::BtcTree& btc() const {
    return cmp_.getProtectingBlockTree().btc();
  }

  const PopForkComparator& getComparator() const { return cmp_; }
  const AltChainParams& getParams() const { return *alt_config_; }
  PayloadsStorage& getStorage() { return storage_; }
  const PayloadsStorage& getStorage() const { return storage_; }

  std::string toPrettyString(size_t level = 0) const;

  using base::setState;
  bool setState(index_t& to, ValidationState& state) override;

  void overrideTip(index_t& to) override;

  void removeSubtree(index_t& toRemove) override;
  using base::removeLeaf;
  using base::removeSubtree;

  void payloadsToCommands(const ATV& atv,
                          const AltBlock& containing,
                          std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VTB& vtb, std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VbkBlock& block,
                          std::vector<CommandPtr>& commands);

 protected:
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;
  PopRewards rewards_;
  PayloadsStorage& storage_;

  void determineBestChain(index_t& candidate, ValidationState& state) override;

  bool addPayloads(index_t& index,
                   PopData& payloads,
                   ValidationState& state,
                   bool continueOnInvalid = false);

  void setTipContinueOnInvalid(index_t& to);

  void removeAllPayloads(index_t& index);
};

template <>
bool checkBlockTime(const BlockIndex<AltBlock>& prev,
                    const AltBlock& block,
                    ValidationState& state,
                    const AltChainParams& params);

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands(
    const typename AltTree::index_t& index, AltTree& tree);

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

inline void PrintTo(const AltTree& tree, std::ostream* os) {
  *os << tree.toPrettyString();
}

inline uint8_t getBlockProof(const AltBlock&) { return 0; }

}  // namespace altintegration

#endif  // !
