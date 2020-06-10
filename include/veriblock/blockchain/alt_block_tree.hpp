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
  using hash_t = typename AltBlock::hash_t;
  using alt_payloads_t = AltPayloads;
  using vbk_payloads_t = VTB;

  using PopForkComparator = PopAwareForkResolutionComparator<AltBlock,
                                                             AltChainParams,
                                                             VbkBlockTree,
                                                             AltTree>;
  using determine_best_chain_f = std::function<int(index_t&, index_t&)>;

  virtual ~AltTree() = default;

  AltTree(const alt_config_t& alt_config,
          const vbk_config_t& vbk_config,
          const btc_config_t& btc_config)
      : alt_config_(&alt_config),
        vbk_config_(&vbk_config),
        btc_config_(&btc_config),
        cmp_(std::make_shared<VbkBlockTree>(vbk_config, btc_config),
             vbk_config,
             alt_config),
        rewards_(alt_config) {}

  //! before any use, bootstrap the three with ALT bootstrap block.
  //! may return false, if bootstrap block is invalid
  bool bootstrap(ValidationState& state);

  bool acceptBlock(const AltBlock& block, ValidationState& state);

  void removePayloads(index_t& index,
                      const std::vector<alt_payloads_t>& alt_payloads,
                      const std::vector<vbk_payloads_t>& vbk_payloads,
                      const std::vector<VbkBlock>& context);

  void removePayloads(const AltBlock::hash_t& containing,
                      const std::vector<alt_payloads_t>& alt_payloads,
                      const std::vector<vbk_payloads_t>& vbk_payloads,
                      const std::vector<VbkBlock>& context);

  bool addPayloads(index_t& index,
                   const std::vector<alt_payloads_t>& alt_payloads,
                   const std::vector<vbk_payloads_t>& vbk_payloads,
                   const std::vector<VbkBlock>& context,
                   ValidationState& state);

  bool addPayloads(const AltBlock::hash_t& containing,
                   const std::vector<alt_payloads_t>& alt_payloads,
                   const std::vector<vbk_payloads_t>& vbk_payloads,
                   const std::vector<VbkBlock>& context,
                   ValidationState& state);

  bool addPayloads(const AltBlock& containing,
                   const std::vector<alt_payloads_t>& alt_payloads,
                   const std::vector<vbk_payloads_t>& vbk_payloads,
                   const std::vector<VbkBlock>& context,
                   ValidationState& state) {
    return addPayloads(
        containing.hash, alt_payloads, vbk_payloads, context, state);
  }

  int comparePopScore(const AltBlock::hash_t& hleft,
                      const AltBlock::hash_t& hright);

  /**
   * Calculate payouts for the altchain tip
   * @return map with reward recipient as a key and reward amount as a value
   */
  std::map<std::vector<uint8_t>, int64_t> getPopPayout(
      const AltBlock::hash_t& tip, ValidationState& state);

  bool setState(const AltBlock::hash_t& block, ValidationState& state) {
    auto* index = getBlockIndex(block);
    if (!index) {
      return false;
    }
    return setState(*index, state);
  }

  bool setState(index_t& index, ValidationState& state) {
    return this->setTip(index, state);
  }

  bool validatePayloads(const AltBlock& block,
                        const alt_payloads_t& p,
                        const std::vector<VbkBlock>& context,
                        ValidationState& state);

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const alt_payloads_t& p,
                        const std::vector<VbkBlock>& context,
                        ValidationState& state);

  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const VbkBlockTree::BtcTree& btc() const {
    return cmp_.getProtectingBlockTree().btc();
  }

  const PopForkComparator& getComparator() const { return cmp_; }

  const AltChainParams& getParams() const { return *alt_config_; }

  std::string toPrettyString(size_t level = 0) const;

 protected:
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;
  PopRewards rewards_;

  index_t* insertBlockHeader(const AltBlock& block);

  void payloadsToCommands(const std::vector<alt_payloads_t>& alt_payloads,
                          const std::vector<vbk_payloads_t>& vbk_payloads,
                          const std::vector<VbkBlock>& context,
                          std::vector<CommandPtr>& commands);

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          ValidationState& state,
                          bool isBootstrap = false) override;

  bool setTip(index_t& to,
              ValidationState& state,
              bool skipSetState = false) override;
};

}  // namespace altintegration

#endif  // !
