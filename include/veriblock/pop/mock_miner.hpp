// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_2_HPP
#define ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_2_HPP

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "blockchain/blocktree.hpp"
#include "blockchain/miner.hpp"
#include "blockchain/pop/vbk_block_tree.hpp"
#include "bootstraps.hpp"
#include "entities/atv.hpp"
#include "entities/btcblock.hpp"
#include "entities/merkle_tree.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vbktx.hpp"
#include "entities/vtb.hpp"
#include "storage/inmem_payloads_provider.hpp"

namespace altintegration {


//! @private
template <typename T>
std::vector<typename T::hash_t> hashAll(const std::vector<T>& txs) {
  std::vector<typename T::hash_t> hashes(txs.size());
  for (size_t i = 0; i < hashes.size(); i++) {
    hashes[i] = txs[i].getHash();
  }
  return hashes;
}

//! @private
class MockMiner {
 public:
  using btc_block_tree = BlockTree<BtcBlock, BtcChainParams>;
  using vbk_block_tree = VbkBlockTree;

 public:
  PopData endorseAltBlock(const PublicationData& publicationData,
                          const VbkBlock::hash_t& lastKnownVbkBlockHash);

  VTB endorseVbkBlock(const VbkBlock& publishedBlock,
                      const BtcBlock::hash_t& lastKnownBtcBlockHash);

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash);

  PopData createPopDataEndorsingAltBlock(
      const VbkBlock& blockOfProof,
      const VbkTx& transaction,
      const VbkBlock::hash_t& lastKnownVbkBlockHash) const;

  ATV createATV(const VbkBlock& blockOfProof, const VbkTx& transaction) const;

  VbkTx createVbkTxEndorsingAltBlock(
      const PublicationData& publicationData) const;

  VTB createVTB(const VbkBlock& containingBlock,
                const VbkPopTx& transaction) const;

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& blockOfProof,
      const BtcTx& transaction,
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash) const;

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) const;

  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const std::vector<VbkTx>& transactions);
  BlockIndex<VbkBlock>* mineVbkBlocks(
      size_t amount, const std::vector<VbkPopTx>& transactions);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip,
                                      const std::vector<VbkTx>& transactions);
  BlockIndex<VbkBlock>* mineVbkBlocks(
      size_t amount,
      const BlockIndex<VbkBlock>& tip,
      const std::vector<VbkPopTx>& transactions);

  BlockIndex<BtcBlock>* mineBtcBlocks(
      size_t amount, const std::vector<BtcTx>& transactions = {});
  BlockIndex<BtcBlock>* mineBtcBlocks(
      size_t amount,
      const BlockIndex<BtcBlock>& tip,
      const std::vector<BtcTx>& transactions = {});

  const BlockIndex<VbkBlock>* vbkTip() const;
  const BlockIndex<BtcBlock>* btcTip() const;

  const BlockIndex<VbkBlock>* getVbkBlockIndex(
      const VbkBlock::hash_t& hash) const;
  const BlockIndex<BtcBlock>* getBtcBlockIndex(
      const BtcBlock::hash_t& hash) const;

  std::vector<VTB> getVTBs(const VbkBlock& block) const;
  const std::unordered_map<VbkBlock::hash_t, std::vector<VTB>>& getAllVTBs()
      const;

  vbk_block_tree& vbk() { return vbk_tree_; }
  btc_block_tree& btc() { return btc_tree_; }
  const vbk_block_tree& vbk() const { return vbk_tree_; }
  const btc_block_tree& btc() const { return btc_tree_; }
  const VbkChainParams& vbkParams() const { return vbk_params_; }
  const BtcChainParams& btcParams() const { return btc_params_; }

  MockMiner() {
    ValidationState state;
    bool ret = btc_tree_.bootstrapWithGenesis(GetRegTestBtcBlock(), state);
    VBK_ASSERT(ret);
    ret = vbk_tree_.bootstrapWithGenesis(GetRegTestVbkBlock(), state);
    VBK_ASSERT_MSG(ret, state.toString());
  }

  InmemPayloadsProvider& getPayloadsProvider() { return payloads_provider_; }

 private:
  template <typename BlockTree, typename Block>
  static std::vector<Block> getBlocks(
      BlockTree& tree,
      const Block& tip,
      const typename Block::hash_t& lastKnownHash);

  template <typename BlockTree, typename Block>
  static BlockIndex<Block>* acceptBlock(BlockTree& tree, const Block& block);

  template <typename Block, typename Tx>
  BlockIndex<Block>* mineBlocks(size_t amount,
                                const BlockIndex<Block>& tip,
                                const std::vector<Tx>& transactions);

  BlockIndex<VbkBlock>* mineBlock(const BlockIndex<VbkBlock>& tip,
                                  const std::vector<VbkTx>& transactions);

  BlockIndex<VbkBlock>* mineBlock(const BlockIndex<VbkBlock>& tip,
                                  const std::vector<VbkPopTx>& transactions);

  BlockIndex<BtcBlock>* mineBlock(const BlockIndex<BtcBlock>& tip,
                                  const std::vector<BtcTx>& transactions);

  bool saveVTBs(BlockIndex<VbkBlock>* blockIndex,
                const std::vector<VbkPopTx>& transactions);

  VbkMerklePath getMerklePath(const VbkBlock& block,
                              const uint256& txHash) const;
  MerklePath getMerklePath(const BtcBlock& block, const uint256& txHash) const;

  BtcChainParamsRegTest btc_params_{};
  VbkChainParamsRegTest vbk_params_{};
  InmemPayloadsProvider payloads_provider_;
  PayloadsIndex payloads_index_;

  Miner<BtcBlock, BtcChainParams> btc_miner_ =
      Miner<BtcBlock, BtcChainParams>(btc_params_);
  Miner<VbkBlock, VbkChainParams> vbk_miner_ =
      Miner<VbkBlock, VbkChainParams>(vbk_params_);

  vbk_block_tree vbk_tree_{
      vbk_params_, btc_params_, payloads_provider_, payloads_index_};
  btc_block_tree& btc_tree_ = vbk_tree_.btc();

  std::unordered_map<VbkBlock::hash_t, std::vector<VTB>> vtbs_;
  std::unordered_map<VbkBlock::hash_t, VbkMerkleTree> vbk_merkle_trees_;
  std::unordered_map<BtcBlock::hash_t, BtcMerkleTree> btc_merkle_trees_;
};

}  // namespace altintegration

#endif
