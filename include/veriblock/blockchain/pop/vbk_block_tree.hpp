#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {

struct VbkBlockTree;

using VbkPopForkResolutionComparator =
    PopAwareForkResolutionComparator<VbkBlockTree,
                                     VbkBlock,
                                     VbkChainParams,
                                     BlockTree<BtcBlock, BtcChainParams>,
                                     BtcEndorsement>;

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams>,
                      private VbkPopForkResolutionComparator {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp,
               const BtcTree& btc,
               const EndorsementRepository<BtcEndorsement>& e,
               const PayloadsRepository& p)
      : VbkTree(vbkp), VbkPopForkResolutionComparator(*this, e, p, btc, vbkp) {}

  BtcTree& btc() { return this->getProtectingBlockTree(); }
  const BtcTree& btc() const { return this->getProtectingBlockTree(); }

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override {
    if (!VbkTree::bootstrapWithChain(startHeight, chain, state)) {
      return state.addStackFunction("VbkTree::bootstrapWithChain");
    }

    if (!this->setState(*getBestChain().tip(), state)) {
      return state.addStackFunction("VbkTree::bootstrapWithChain");
    }

    return true;
  }

  bool bootstrapWithGenesis(ValidationState& state) override {
    if (!VbkTree::bootstrapWithGenesis(state)) {
      return state.addStackFunction("VbkTree::bootstrapWithGenesis");
    }

    if (!this->setState(*getBestChain().tip(), state)) {
      return state.addStackFunction("VbkTree::bootstrapWithChain");
    }

    return true;
  }

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;
};

template <>
bool addPayloads(VbkBlockTree& tree,
                 const Payloads& payloads,
                 ValidationState& state);

template <>
void removePayloads(VbkBlockTree& tree, const Payloads& payloads);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
