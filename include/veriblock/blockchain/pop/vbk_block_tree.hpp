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
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/storage/endorsement_storage.hpp>

namespace altintegration {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using index_t = VbkTree::index_t;
  using endorsement_t = typename index_t::endorsement_t;
  using payloads_t = typename index_t::payloads_t;
  using PopForkComparator = PopAwareForkResolutionComparator<VbkBlock,
                                                             VbkChainParams,
                                                             BtcTree,
                                                             VbkBlockTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp, const BtcChainParams& btcp)
      : VbkTree(vbkp), cmp_(std::make_shared<BtcTree>(btcp), btcp, vbkp), storage_() {}

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  PopForkComparator& getComparator() { return cmp_; }
  const PopForkComparator& getComparator() const { return cmp_; }

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override;

  bool bootstrapWithGenesis(ValidationState& state) override;

  bool addPayloads(const VbkBlock::hash_t& hash,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  void removePayloads(const hash_t& hash,
                      const std::vector<payloads_t>& payloads);

  void removePayloads(const block_t& block,
                      const std::vector<payloads_t>& payloads);

  void payloadsToCommands(const typename VbkBlockTree::payloads_t& p,
                          std::vector<CommandPtr>& commands);

  bool operator==(const VbkBlockTree& o) const {
    return cmp_ == o.cmp_ && VbkTree::operator==(o);
  }

  bool operator!=(const VbkBlockTree& o) const { return !operator==(o); }

  std::string toPrettyString(size_t level = 0) const;

 private:
  bool setTip(index_t& to,
              ValidationState& state,
              bool isBootstrap = false) override;

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          ValidationState& state,
                          bool isBootstrap = false) override;

  PopForkComparator cmp_;
  EndorsementStorage<payloads_t> storage_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
