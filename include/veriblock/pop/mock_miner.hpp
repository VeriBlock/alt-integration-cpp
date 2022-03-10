// Copyright (c) 2019-2022 Xenios SEZC
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
#include "storage/adaptors/block_provider_impl.hpp"
#include "storage/adaptors/inmem_storage_impl.hpp"
#include "storage/adaptors/payloads_provider_impl.hpp"

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
      const VbkTx& tx,
      const VbkBlock::hash_t& lastKnownVbkBlockHash) const;

  ATV createATV(const VbkBlock& blockOfProof, const VbkTx& tx) const;

  VbkTx createVbkTxEndorsingAltBlockWithSourceAmount(
      const PublicationData& publicationData, const Coin& sourceAmount) const;

  VbkTx createVbkTxEndorsingAltBlock(
      const PublicationData& publicationData) const;

  VTB createVTB(const VbkBlock& containingBlock, const VbkPopTx& tx) const;

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& blockOfProof,
      const BtcTx& tx,
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash) const;

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) const;

  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const std::vector<VbkTx>& txs,
                                      const std::vector<VbkPopTx>& pop_txs);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const std::vector<VbkTx>& txs);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const std::vector<VbkPopTx>& pop_txs);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip,
                                      const std::vector<VbkTx>& txs,
                                      const std::vector<VbkPopTx>& pop_txs);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip,
                                      const std::vector<VbkTx>& txs);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount,
                                      const BlockIndex<VbkBlock>& tip,
                                      const std::vector<VbkPopTx>& pop_txs);

  BlockIndex<BtcBlock>* mineBtcBlocks(size_t amount,
                                      const std::vector<BtcTx>& txs = {});
  BlockIndex<BtcBlock>* mineBtcBlocks(size_t amount,
                                      const BlockIndex<BtcBlock>& tip,
                                      const std::vector<BtcTx>& txs = {});

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

  MockMiner(const AltChainParams& alt_config,
            const VbkChainParams& vbk_config,
            const BtcChainParams& btc_config)
      : alt_params_(alt_config),
        vbk_params_(vbk_config),
        btc_params_(btc_config) {
    btc_tree_.bootstrapWithGenesis(GetRegTestBtcBlock());
    vbk_tree_.bootstrapWithGenesis(GetRegTestVbkBlock());
  }

  adaptors::InmemStorageImpl& getStorage() { return storage_; }

 private:
  template <typename BlockTree, typename Block>
  static std::vector<Block> getBlocks(
      BlockTree& tree,
      const Block& tip,
      const typename Block::hash_t& lastKnownHash);

  template <typename BlockTree, typename Block>
  static BlockIndex<Block>* acceptBlock(BlockTree& tree, const Block& block);

  BlockIndex<VbkBlock>* mineBlocks(size_t amount,
                                   const BlockIndex<VbkBlock>& tip,
                                   const std::vector<VbkTx>& txs,
                                   const std::vector<VbkPopTx>& pop_txs);

  BlockIndex<BtcBlock>* mineBlocks(size_t amount,
                                   const BlockIndex<BtcBlock>& tip,
                                   const std::vector<BtcTx>& txs);

  BlockIndex<VbkBlock>* mineBlock(const BlockIndex<VbkBlock>& tip,
                                  const std::vector<VbkTx>& txs);

  BlockIndex<VbkBlock>* mineBlock(const BlockIndex<VbkBlock>& tip,
                                  const std::vector<VbkPopTx>& txs);

  BlockIndex<VbkBlock>* mineBlock(const BlockIndex<VbkBlock>& tip,
                                  const std::vector<VbkTx>& txs,
                                  const std::vector<VbkPopTx>& pop_txs);

  BlockIndex<BtcBlock>* mineBlock(const BlockIndex<BtcBlock>& tip,
                                  const std::vector<BtcTx>& txs);

  bool saveVTBs(BlockIndex<VbkBlock>* blockIndex,
                const std::vector<VbkPopTx>& txs);

  VbkMerklePath getMerklePath(const VbkBlock& block,
                              const uint256& txHash,
                              VbkMerkleTree::TreeIndex treeIndex) const;
  MerklePath getMerklePath(const BtcBlock& block, const uint256& txHash) const;

  const AltChainParams& alt_params_;
  const VbkChainParams& vbk_params_;
  const BtcChainParams& btc_params_;
  adaptors::InmemStorageImpl storage_{};
  adaptors::PayloadsStorageImpl payloads_provider_{storage_};
  adaptors::BlockReaderImpl block_provider_{storage_, alt_params_};

  Miner<BtcBlock, BtcChainParams> btc_miner_{btc_params_};
  Miner<VbkBlock, VbkChainParams> vbk_miner_{vbk_params_};

  vbk_block_tree vbk_tree_{vbk_params_,
                           btc_params_,
                           payloads_provider_,
                           block_provider_};
  btc_block_tree& btc_tree_ = vbk_tree_.btc();

  std::unordered_map<VbkBlock::hash_t, std::vector<VTB>> vtbs_;
  std::unordered_map<VbkBlock::hash_t, VbkMerkleTree> vbk_merkle_trees_;
  std::unordered_map<BtcBlock::hash_t, BtcMerkleTree> btc_merkle_trees_;
};

}  // namespace altintegration

#endif
