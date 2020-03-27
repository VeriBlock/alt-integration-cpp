#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>

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

  const PopForkComparator& getComparator() const { return cmp_; }

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
                   ValidationState& state,
                   StateChange* change = nullptr) {
    index_t* index;
    if (!validateAndAddBlock(block, state, true, &index)) {
      return state.addStackFunction("VbkTree::acceptBlock");
    }

    assert(index != nullptr);

    if (!cmp_.addAllPayloads(*index, payloads, state)) {
      return state.Invalid("vbk-invalid-pop-" + state.GetRejectReason(),
                           state.GetDebugMessage());
    }

    for (const auto& p : payloads) {
      index->containingPayloads.push_back(p.getId());
      BtcEndorsement e = BtcEndorsement::fromContainer(p);
      index->containingEndorsements[e.id] = std::make_shared<BtcEndorsement>(e);
    }

    // save payloads on disk
    if (change) {
      for (const auto& payload : payloads) {
        change->saveVbkPayloads(payload);
      }
    }

    bool ret = cmp_.setState(*index, state);
    assert(ret && "this state was validated previously");
    (void)ret;

    determineBestChain(activeChain_, *index);

    return true;
  }

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;

  PopForkComparator cmp_;
};

template <>
bool PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::addPayloads(const VTB& payloads,
                                                  ValidationState& state);

template <>
void PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::removePayloads(const VTB& payloads);

template <>
bool checkEndorsement(BlockIndex<VbkBlock>& currentBlock,
                      const BtcEndorsement& e,
                      const VbkChainParams& params,
                      ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
