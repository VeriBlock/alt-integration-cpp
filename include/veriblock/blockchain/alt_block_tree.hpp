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
#include "veriblock/entities/popdata.hpp"
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

  void removePayloads(index_t& index, const PopData& popData);

  void removePayloads(const AltBlock::hash_t& containing,
                      const PopData& popData);

  template <typename pop_t>
  void removePayloads(index_t& index,
                      const std::vector<pop_t>& payloads,
                      bool set_state = true) {
    VBK_LOG_INFO("%s remove %d %s payloads from %s",
                 block_t::name(),
                 payloads.size(),
                 pop_t::name(),
                 index.toShortPrettyString());

    if (set_state) {
      if (!index.pprev) {
        // we do not add payloads to genesis block, therefore we do not have to
        // remove them
        return;
      }

      bool isOnActiveChain = activeChain_.contains(&index);
      if (isOnActiveChain) {
        VBK_ASSERT(index.pprev && "can not remove payloads from genesis block");
        ValidationState dummy;
        bool ret = setTip(*index.pprev, dummy, false);
        VBK_ASSERT(ret);
      }
    }

    std::vector<typename pop_t::id_t> pids(payloads.size());
    for (size_t i = 0; i < payloads.size(); ++i) {
      pids[i] = payloads[i].getId();
    }

    auto& payloadIds = index.getPayloadIds<pop_t, typename pop_t::id_t>();

    for (const auto& pid : pids) {
      auto it = std::find(payloadIds.begin(), payloadIds.end(), pid);
      if (it == payloadIds.end()) {
        // TODO: error message
        continue;
      }

      auto stored_payload = storagePayloads_.loadPayloads<pop_t>(pid);
      if (!stored_payload.valid) {
        revalidateSubtree(index, BLOCK_FAILED_POP, false);
      }

      payloadIds.erase(it);
      // TODO: do we want to erase payloads from repository?
    }
  }

  bool addPayloads(index_t& index,
                   const PopData& popData,
                   ValidationState& state);

  template <
      typename pop_t,
      typename = typename std::enable_if<
          std::is_same<pop_t, ATV>::value || std::is_same<pop_t, VTB>::value ||
          std::is_same<pop_t, VbkBlock>::value>::type>
  bool addPayloads(index_t& index,
                   const std::vector<pop_t>& payloads,
                   ValidationState& state) {
    VBK_LOG_INFO("%s add %d alt payloads to block %s",
                 block_t::name(),
                 payloads.size(),
                 index.toShortPrettyString());

    if (!index.pprev) {
      return state.Invalid(
          block_t::name() + "-bad-containing-prev",
          "It is forbidden to add payloads to bootstrap block");
    }

    if (!index.isValid()) {
      return state.Invalid(block_t::name() + "-bad-chain",
                           "Containing block has been marked as invalid");
    }

    bool isOnActiveChain = activeChain_.contains(&index);
    if (isOnActiveChain) {
      ValidationState dummy;
      bool ret = setTip(*index.pprev, dummy, false);
      VBK_ASSERT(ret);
    }

    auto& payloadIds = index.getPayloadIds<pop_t, typename pop_t::id_t>();

    std::set<typename pop_t::id_t> existingPids(payloadIds.begin(),
                                                payloadIds.end());

    for (const auto& p : payloads) {
      auto pid = p.getId();
      if (!existingPids.insert(pid).second) {
        return state.Invalid(
            block_t::name() + "-duplicate-payloads",
            fmt::sprintf("Containing block=%s already contains payload %s.",
                         index.toPrettyString(),
                         pid.toHex()));
      }

      payloadIds.push_back(pid);
      storagePayloads_.savePayloads(p);
    }

    return true;
  }

  bool addPayloads(const AltBlock::hash_t& containing,
                   const PopData& popData,
                   ValidationState& state);

  bool addPayloads(const AltBlock& containing,
                   const PopData& popData,
                   ValidationState& state) {
    return addPayloads(containing.hash, popData, state);
  }

  void payloadsToCommands(const ATV& atv,
                          const AltBlock& containing,
                          std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VTB& vtb, std::vector<CommandPtr>& commands);

  void payloadsToCommands(const VbkBlock& block,
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
                        const PopData& popData,
                        ValidationState& state);

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const PopData& popData,
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
  const PayloadsStorage getStoragePayloads() const { return storagePayloads_; }

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

template <>
std::vector<CommandGroup> PayloadsStorage::loadCommands<AltTree>(
    const typename AltTree::index_t& index, AltTree& tree);

template <>
void PopStorage::saveBlocks(
    const std::unordered_map<typename AltBlock::prev_hash_t,
                             std::shared_ptr<BlockIndex<AltBlock>>>& blocks);

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

uint8_t getBlockProof(const AltBlock&);

}  // namespace altintegration

#endif  // !
