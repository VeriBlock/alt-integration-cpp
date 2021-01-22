// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php. 

#ifndef ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_2_HPP
#define ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_2_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/bootstraps.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/merkle_tree.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/storage/inmem_payloads_provider.hpp"

namespace altintegration {

template <typename T>
std::vector<typename T::hash_t> hashAll(const std::vector<T>& txes) {
  std::vector<typename T::hash_t> ret;
  ret.reserve(txes.size());
  std::transform(
      txes.begin(), txes.end(), std::back_inserter(ret), [](const T& tx) {
        return tx.getHash();
      });
  return ret;
}

//! @private
class MockMiner2 {
 public:
  using btc_block_t = BtcBlock;
  using btc_params_t = BtcChainParams;
  using btc_block_tree = BlockTree<btc_block_t, btc_params_t>;
  using vbk_block_tree = VbkBlockTree;

 public:
  std::vector<BtcTx> btcmempool;
  std::vector<VbkPopTx> vbkmempool;
  std::unordered_map<VbkBlock::hash_t, std::vector<VTB>> vbkPayloads;

  VbkPopTx endorseVbkBlock(const VbkBlock& publishedBlock,
                           const BtcBlock::hash_t& lastKnownBtcBlockHash);

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock);

  VbkPopTx createVbkPopTxEndorsingVbkBlock(const BtcBlock& containingBlock,
                                           const BtcTx& containingTx,
                                           const VbkBlock& publishedBlock,
                                           const BtcBlock::hash_t& lastKnownBtcBlockHash);

  VbkTx createVbkTxEndorsingAltBlock(const PublicationData& publicationData);

  // TODO: Rename after getting rid of copy
  BtcBlock mineBtcBlocks_(size_t amount);
  BtcBlock mineBtcBlocks(size_t amount, const BlockIndex<BtcBlock>& tip);

  BtcBlock mineBtcBlock(const BlockIndex<BtcBlock>& tip);

  BtcBlock mineBtcBlock(const std::vector<BtcTx>& transactions);
  BtcBlock mineBtcBlock(const std::vector<BtcTx>& transactions,
                        const BlockIndex<BtcBlock>& tip);
  // TODO: Rename after getting rid of copy
  VbkBlock mineVbkBlocks_(size_t amount);
  VbkBlock mineVbkBlocks(size_t amount, const BlockIndex<VbkBlock>& tip);

  VbkBlock mineVbkBlock(const BlockIndex<VbkBlock>& tip);

  VbkBlock mineVbkBlock(const std::vector<VbkPopTx>& transactions);
  VbkBlock mineVbkBlock(const std::vector<VbkPopTx>& transactions,
                        const BlockIndex<VbkBlock>& tip);

  VbkBlock mineVbkBlock(const std::vector<VbkTx>& transactions);
  VbkBlock mineVbkBlock(const std::vector<VbkTx>& transactions,
                        const BlockIndex<VbkBlock>& tip);

  std::vector<VTB> getVTBs(const VbkBlock& block) const;

  std::vector<ATV> getATVs(const VbkBlock& block) const;

  BlockIndex<BtcBlock>* getBtcBlockIndex(const BtcBlock& block);
  BlockIndex<BtcBlock>* getBtcBlockIndex(const BtcBlock::hash_t& blockHash);

  BlockIndex<VbkBlock>* getVbkBlockIndex(const VbkBlock& block);
  BlockIndex<VbkBlock>* getVbkBlockIndex(const VbkBlock::hash_t& hash);

  const BtcBlock getBtcTip() const;
  const BlockIndex<BtcBlock>* getBtcTipIndex() const;

  const VbkBlock getVbkTip() const;
  const BlockIndex<VbkBlock>* getVbkTipIndex() const;

  btc_block_tree& btc() { return vbktree.btc(); }
  vbk_block_tree& vbk() { return vbktree; }
  const btc_block_tree& btc() const { return vbktree.btc(); }
  const vbk_block_tree& vbk() const { return vbktree; }
  const VbkChainParams& getVbkParams() const { return vbk_params; }
  const BtcChainParams& getBtcParams() const { return btc_params; }

  // TODO: Inline and get rid of
  BlockIndex<BtcBlock>* mineBtcBlocks(const BlockIndex<BtcBlock>& tip,
                                      size_t amount);
  BlockIndex<BtcBlock>* mineBtcBlocks(size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                      size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                      const std::vector<VbkPopTx>& poptxs);
  VbkBlock applyVTB(const VbkPopTx& tx);
  VbkBlock applyVTB(const VbkBlock::hash_t& tip, const VbkPopTx& tx);
  VbkBlock applyVTBs(const std::vector<VbkPopTx>& txes);
  VbkBlock applyVTBs(const BlockIndex<VbkBlock>& tip,
                     const std::vector<VbkPopTx>& txes);
  ATV applyATV(const VbkTx& transaction);
  std::vector<ATV> applyATVs(const std::vector<VbkTx>& transactions);

  MockMiner2() {
    ValidationState state;
    bool ret = vbktree.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), state);
    VBK_ASSERT(ret);
    ret = vbktree.bootstrapWithGenesis(GetRegTestVbkBlock(), state);
    VBK_ASSERT_MSG(ret, state.toString());
  }

  InmemPayloadsProvider& getPayloadsProvider() { return payloadsProvider; }

 private:
  BtcChainParamsRegTest btc_params{};
  VbkChainParamsRegTest vbk_params{};
  InmemPayloadsProvider payloadsProvider;
  PayloadsIndex payloadsIndex;

  Miner<BtcBlock, BtcChainParams> btc_miner =
      Miner<BtcBlock, BtcChainParams>(btc_params);
  Miner<VbkBlock, VbkChainParams> vbk_miner =
      Miner<VbkBlock, VbkChainParams>(vbk_params);

  VbkBlockTree vbktree{vbk_params, btc_params, payloadsProvider, payloadsIndex};

  std::unordered_map<BtcBlock::hash_t, std::vector<BtcTx>> btcTxs;
  std::unordered_map<VbkBlock::hash_t, std::vector<VbkPopTx>> vbkPopTxs;
  std::unordered_map<VbkBlock::hash_t, std::vector<VbkTx>> vbkTxs;

  void savePayloads(const VbkBlock& block);
};

}  // namespace altintegration

#endif
