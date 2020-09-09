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
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/rewards/poprewards.hpp"
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

  bool bootstrap(ValidationState& state);

  bool acceptBlockHeader(const AltBlock& block, ValidationState& state);

  void acceptBlock(const hash_t& block, const PopData& payloads);

  void acceptBlock(index_t& index, const PopData& payloads);

  std::vector<const index_t*> getConnectedTipsAfter(const index_t& index) const;

  signals::Signal<void(index_t& index, ValidationState&)>
      onInvalidBlockConnected;

  signals::Signal<void(index_t& index)> onBlockConnected;

  bool addPayloads(const hash_t& block,
                   const PopData& popData,
                   ValidationState& state);
  bool addPayloads(index_t& index, PopData& payloads, ValidationState& state);

  bool loadBlock(const index_t& index, ValidationState& state) override;
  bool loadTip(const hash_t& hash, ValidationState& state) override;
  int comparePopScore(const AltBlock::hash_t& A, const AltBlock::hash_t& B);
  std::map<std::vector<uint8_t>, int64_t> getPopPayout(const hash_t& tip);

  bool setState(index_t& to, ValidationState& state) override;
  using base::removeSubtree;
  using base::setState;
  void removeSubtree(index_t& toRemove) override;
  void removePayloads(const hash_t& hash);
  void filterInvalidPayloads(PopData& pop);

  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }

  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const VbkBlockTree::BtcTree& btc() const {
    return cmp_.getProtectingBlockTree().btc();
  }
  const PopForkComparator& getComparator() const { return cmp_; }
  const AltChainParams& getParams() const { return *alt_config_; }
  PayloadsIndex& getPayloadsIndex() { return payloadsIndex_; }
  const PayloadsIndex& getPayloadsIndex() const { return payloadsIndex_; }
  PayloadsProvider& getPayloadsProvider() { return payloadsProvider_; }
  const PayloadsProvider& getPayloadsProvider() const {
    return payloadsProvider_;
  }
  std::string toPrettyString(size_t level = 0) const;

  void overrideTip(index_t& to) override;

  using base::removeLeaf;

 protected:
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;
  PopRewards rewards_;
  PayloadsIndex payloadsIndex_;
  PayloadsProvider& payloadsProvider_;

  void determineBestChain(index_t& candidate, ValidationState& state) override;

  void setPayloads(index_t& index, const PopData& payloads);
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

//! @private
inline uint8_t getBlockProof(const AltBlock&) { return 0; }

}  // namespace altintegration

#endif  // !
