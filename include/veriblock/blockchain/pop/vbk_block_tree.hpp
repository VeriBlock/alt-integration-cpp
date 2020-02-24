#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/storage/endorsements_repository.hpp>

namespace VeriBlock {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(BtcTree& btc,
               std::shared_ptr<EndorsementsRepository> erepo,
               std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> blockrepo,
               std::shared_ptr<VbkChainParams> params)
      : VbkTree(std::move(blockrepo), std::move(params)),
        erepo_(std::move(erepo)),
        btc_(btc) {}

 private:
  std::shared_ptr<EndorsementsRepository> erepo_;
  BtcTree& btc_;

  void determineBestChain(Chain<block_t>& currentBest,
                          index_t& indexNew) override;

 protected:
  std::vector<ProtoKeystoneContext> getProtoKeystoneContext(
      const Chain<VbkBlock>& chain);

  std::vector<KeystoneContext> getKeystoneContext(
      const std::vector<ProtoKeystoneContext>& chain);

  bool isCrossedKeystoneBoundary(const index_t& bottom, const index_t& tip);
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
