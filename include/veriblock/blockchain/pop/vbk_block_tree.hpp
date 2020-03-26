#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>
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

    auto* tip = getBestChain().tip();
    if (!cmp_.setState(*tip, state)) {
      return state.addStackFunction("VbkTree::bootstrapWithGenesis");
    }

    return true;
  }

  bool acceptBlock(const block_t& block,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state) {
    index_t* index;
    if (!validateBlock(block, state, true, &index)) {
      return state.addStackFunction("VbkTree::acceptBlock");
    }

    assert(index != nullptr);

    cmp_.getPayloadsRepository()
    if (!cmp_.setState(*index, state)) {
      return state.addStackFunction("VbkTree::acceptBlock");
    }

    // add payloads to block index
  }

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;

  PopForkComparator cmp_;
};

template <>
bool PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::addPayloads(BlockIndex<VbkBlock>* index, const VTB& payloads,
                                                  ValidationState& state);

template <>
void PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::removePayloads(BlockIndex<VbkBlock>* index, const VTB& payloads);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
