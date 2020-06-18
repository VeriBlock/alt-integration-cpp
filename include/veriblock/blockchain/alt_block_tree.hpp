// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <veriblock/storage/payloads_storage.hpp>
#include <veriblock/storage/pop_storage.hpp>

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/base_block_tree.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/payloads.hpp"
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
  using payloads_t = typename AltBlock::payloads_t;
  using pid_t = typename payloads_t::id_t;
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
          PayloadsStorage& storagePayloads)
      : alt_config_(&alt_config),
        vbk_config_(&vbk_config),
        btc_config_(&btc_config),
        cmp_(std::make_shared<VbkBlockTree>(
                 vbk_config, btc_config, storagePayloads),
             vbk_config,
             alt_config,
             storagePayloads),
        rewards_(alt_config),
        storagePayloads_(storagePayloads) {}

  //! before any use, bootstrap the three with ALT bootstrap block.
  //! may return false, if bootstrap block is invalid
  bool bootstrap(ValidationState& state);

  bool acceptBlock(const AltBlock& block, ValidationState& state);

  void removePayloads(index_t& index, const std::vector<pid_t>& pids);

  void removePayloads(const AltBlock::hash_t& containing,
                      const std::vector<pid_t>& pids);

  bool addPayloads(index_t& index,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  bool addPayloads(const AltBlock::hash_t& containing,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  bool addPayloads(const AltBlock& containing,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state) {
    return addPayloads(containing.hash, payloads, state);
  }

  void payloadsToCommands(const payloads_t& p,
                          std::vector<CommandPtr>& commands);

  bool saveToStorage(PopStorage& storage, ValidationState& state);

  bool loadFromStorage(const PopStorage& storage, ValidationState& state);

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
                        const payloads_t& p,
                        ValidationState& state);

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const payloads_t& p,
                        ValidationState& state);

  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const VbkBlockTree::BtcTree& btc() const {
    return cmp_.getProtectingBlockTree().btc();
  }

  const PopForkComparator& getComparator() const { return cmp_; }

  const AltChainParams& getParams() const { return *alt_config_; }

  PayloadsStorage& getStoragePayloads() { return storagePayloads_; }
  const PayloadsStorage& getStoragePayloads() const { return storagePayloads_; }

  std::string toPrettyString(size_t level = 0) const;

 protected:
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;
  PopRewards rewards_;
  PayloadsStorage& storagePayloads_;

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          ValidationState& state,
                          bool isBootstrap = false) override;

  bool setTip(index_t& to,
              ValidationState& state,
              bool skipSetState = false) override;
};

template <typename JsonValue>
JsonValue ToJSON(const BlockIndex<AltBlock>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
  std::vector<uint256> endorsements;
  for (const auto& e : i.containingEndorsements) {
    endorsements.push_back(e.first);
  }
  json::putArrayKV(obj, "containingEndorsements", endorsements);

  std::vector<uint256> endorsedBy;
  for (const auto* e : i.endorsedBy) {
    endorsedBy.push_back(e->id);
  }
  json::putArrayKV(obj, "endorsedBy", endorsedBy);

  json::putIntKV(obj, "status", i.status);

  return obj;
}

template <>
ArithUint256 getBlockProof(const AltBlock&);

}  // namespace altintegration

#endif  // !
