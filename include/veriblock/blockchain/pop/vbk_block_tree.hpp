// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using index_t = VbkTree::index_t;
  using endorsement_t = typename index_t::endorsement_t;
  using context_t = typename index_t::block_t::context_t;
  using PopForkComparator =
      PopAwareForkResolutionComparator<VbkBlock, VbkChainParams, BtcTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp, const BtcChainParams& btcp)
      : VbkTree(vbkp), cmp_(BtcTree(btcp), btcp, vbkp) {}

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  PopForkComparator& getComparator() { return cmp_; }
  const PopForkComparator& getComparator() const { return cmp_; }

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override;

  bool bootstrapWithGenesis(ValidationState& state) override;

  void invalidateBlockByHash(const hash_t& blockHash) override;

  bool addPayloads(const hash_t& containing,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  bool operator==(const VbkBlockTree& o) const {
    if(cmp_ != o.cmp_) {
      return false;
    }

    return VbkTree::operator==(o);
  }

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          bool isBootstrap = false) override;

  PopForkComparator cmp_;
};

template <>
bool processPayloads<VbkBlockTree>(VbkBlockTree& tree,
                                   const VbkBlock::hash_t& index,
                                   const VTB& p,
                                   ValidationState& state,
                                   CommandHistory& history);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
