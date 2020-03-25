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
  using index_t = VbkTree::index_t;

  ~VbkBlockTree() override = default;

  VbkBlockTree(BtcTree& btc,
               std::shared_ptr<EndorsementRepository<BtcEndorsement>> erepo,
               const VbkChainParams& params)
      : VbkTree(params),
        erepo_(std::move(erepo)),
        btc_(btc),
        compare_(this->param_) {}

 private:
  std::shared_ptr<EndorsementRepository<BtcEndorsement>> erepo_;
  BtcTree& btc_;
  ComparePopScore<VbkChainParams> compare_;

  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew) override;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
