#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/state_machine.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using index_t = VbkTree::index_t;
  using PopForkComparator =
      PopAwareForkResolutionComparator<VbkBlock, VbkChainParams, BtcTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& params, PopForkComparator cmp)
      : VbkTree(params), cmp_(std::move(cmp)) {}

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override {
    if (!VbkTree::bootstrapWithChain(startHeight, chain, state)) {
      return state.addStackFunction("VbkTree::bootstrapWithChain");
    }

    if (!cmp_.setState(*getBestChain().tip(), state)) {
      return state.addStackFunction("VbkTree::bootstrapWithChain");
    }

    return true;
  }

  bool bootstrapWithGenesis(ValidationState& state) override {
    if (!VbkTree::bootstrapWithGenesis(state)) {
      return state.addStackFunction("VbkTree::bootstrapWithGenesis");
    }

    if (!cmp_.setState(*getBestChain().tip(), state)) {
      return state.addStackFunction("VbkTree::bootstrapWithGenesis");
    }

    return true;
  }

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;

  PopForkComparator cmp_;
};

template <>
bool BlockTreeStateMachine<VbkBlockTree::BtcTree, BlockIndex<VbkBlock>, VbkChainParams>::
    addPayloads(const VTB& payloads, ValidationState& state);

template <>
void BlockTreeStateMachine<VbkBlockTree::BtcTree, BlockIndex<VbkBlock>, VbkChainParams>::
    removePayloads(const VTB& payloads);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
