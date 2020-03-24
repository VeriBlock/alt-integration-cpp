#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using comparator_t = ComparePopScore<VbkTree, BtcTree, BtcEndorsement>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(comparator_t cmp,
               const VbkChainParams& vbkp,
               const BtcChainParams& btcp)
      : VbkTree(vbkp), btc_(btcp), compare_(cmp) {}

  BtcTree& btc() { return btc_; }
  const BtcTree& btc() const { return btc_; }

 private:
  BtcTree btc_;
  ComparePopScore<VbkTree, BtcTree, BtcEndorsement> compare_;

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;
};

template <>
bool addPayloads(VbkBlockTree& tree,
                 const Payloads& payloads,
                 ValidationState& state);

template <>
bool removePayloads(VbkBlockTree& tree, const Payloads& payloads);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
