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

/**
 * @warning POP Fork Resolution is NOT transitive. If A is better than B, and B
 * is better than C, then A may NOT be better than C.
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
  using determine_best_chain_f = std::function<int(index_t&, index_t&)>;

  virtual ~AltTree() = default;

  AltTree(const alt_config_t& alt_config,
          const vbk_config_t& vbk_config,
          const btc_config_t& btc_config,
          PayloadsStorage& storagePayloads);

  //! before any use, bootstrap the three with ALT bootstrap block.
  //! may return false, if bootstrap block is invalid
  bool bootstrap(ValidationState& state);

  bool acceptBlock(const AltBlock& block, ValidationState& state);

  void removePayloads(index_t& index, const PopData& popData);

  void removePayloads(const AltBlock::hash_t& containing,
                      const PopData& popData);

  bool addPayloads(index_t& index,
                   const PopData& popData,
                   ValidationState& state);

  bool addPayloads(const AltBlock::hash_t& containing,
                   const PopData& popData,
                   ValidationState& state);

  bool addPayloads(const AltBlock& containing,
                   const PopData& popData,
                   ValidationState& state);

  void payloadsToCommands(const ATV& atv,
                          const AltBlock& containing,
                          std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VTB& vtb, std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VbkBlock& block,
                          std::vector<CommandPtr>& commands);

  //! efficiently connect `index` to current tree, loaded from disk
  //! - recovers all pointers (pprev, pnext, endorsedBy)
  //! - does validation of endorsements
  //! - recovers tips array
  //! @invariant NOT atomic.
  bool loadBlock(const index_t& index, ValidationState& state) override;

  bool loadTip(const hash_t& hash, ValidationState& state) override;

  bool operator==(const AltTree& o) const {
    return cmp_ == o.cmp_ && base::operator==(o);
  }

  bool operator!=(const AltTree& o) const { return !operator==(o); }

  int comparePopScore(const AltBlock::hash_t& hleft,
                      const AltBlock::hash_t& hright);

  /**
   * Calculate payouts for the altchain tip
   * @return map with reward recipient as a key and reward amount as a value
   */
  std::map<std::vector<uint8_t>, int64_t> getPopPayout(
      const AltBlock::hash_t& tip, ValidationState& state);

  bool validatePayloads(const AltBlock& block,
                        const PopData& popData,
                        ValidationState& state);

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const PopData& popData,
                        ValidationState& state);

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
  const PayloadsStorage getStorage() const { return storage_; }

  std::string toPrettyString(size_t level = 0) const;

  using base::setState;
  bool setState(index_t& to, ValidationState& state) override;

  void overrideTip(index_t& to) override;

  void removeSubtree(index_t& toRemove) override;
  using base::removeLeaf;
  using base::removeSubtree;

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
};

template <>
void removePayloadsFromIndex(BlockIndex<AltBlock>& index,
                             const CommandGroup& cg);

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
  json::putIntKV(obj, "status", i.status);

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
