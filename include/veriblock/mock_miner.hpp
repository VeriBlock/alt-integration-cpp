#ifndef ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP
#define ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/merkle_tree.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/storage/payloads_repository_inmem.hpp"

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

class MockMiner {
 public:
  using btc_block_t = BtcBlock;
  using btc_params_t = BtcChainParams;
  using btc_block_tree = BlockTree<btc_block_t, btc_params_t>;
  using btc_block_index_t = btc_block_tree::index_t;

  using vbk_block_t = VbkBlock;
  using vbk_block_tree = VbkBlockTree;
  using vbk_block_index_t = vbk_block_tree::index_t;

 public:
  std::vector<BtcTx> btcmempool;
  std::vector<VbkPopTx> vbkmempool;
  std::unordered_map<VbkBlock::hash_t, std::vector<VTB>> vbkPayloads;
  std::unordered_map<AltBlock::hash_t, std::vector<AltPayloads>> altPayloads;

  // TODO: no alt tree yet
  //  VbkTx endorseAltBlock(const PublicationData& publicationData);
  //  ATV generateAndApplyATV(const PublicationData& publicationData,
  //                          const VbkBlock::hash_t& lastKnownVbkBlockHash,
  //                          ValidationState& state);

  BlockIndex<BtcBlock>* mineBtcBlocks(const BlockIndex<BtcBlock>& tip,
                                      size_t amount);
  BlockIndex<BtcBlock>* mineBtcBlocks(size_t amount);

  BlockIndex<VbkBlock>* mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                      size_t amount);
  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount);

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock);

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& containingBlock,
      const BtcTx& containingTx,
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash);

  VbkPopTx endorseVbkBlock(const VbkBlock& publishedBlock,
                           const BtcBlock::hash_t& lastKnownBtcBlockHash,
                           ValidationState& state);

  VbkBlock applyVTB(VbkBlockTree& tree,
                    const VbkPopTx& tx,
                    ValidationState& state);

  VbkBlock applyVTB(const BlockIndex<VbkBlock>& tip,
                    VbkBlockTree& tree,
                    const VbkPopTx& tx,
                    ValidationState& state);

  VbkBlock applyVTBs(VbkBlockTree& tree,
                     const std::vector<VbkPopTx>& txes,
                     ValidationState& state);

  VbkBlock applyVTBs(const BlockIndex<VbkBlock>& tip,
                     VbkBlockTree& tree,
                     const std::vector<VbkPopTx>& txes,
                     ValidationState& state);

  //  void getGeneratedVTBs(const BlockIndex<VbkBlock>& containingBlock,
  //                        std::vector<VTB>& vtbs);

  btc_block_tree& btc() { return vbktree.btc(); }
  vbk_block_tree& vbk() { return vbktree; }
  const VbkChainParams& getVbkParams() const { return vbk_params; }
  const BtcChainParams& getBtcParams() const { return btc_params; }

  MockMiner() {
    bool ret = false;
    ret = vbktree.btc().bootstrapWithGenesis(state_);
    assert(ret);
    ret = vbktree.bootstrapWithGenesis(state_);
    assert(ret);
    (void)ret;
  }

 private:
  BtcChainParamsRegTest btc_params{};
  VbkChainParamsRegTest vbk_params{};

  Miner<BtcBlock, BtcChainParams> btc_miner =
      Miner<BtcBlock, BtcChainParams>(btc_params);
  Miner<VbkBlock, VbkChainParams> vbk_miner =
      Miner<VbkBlock, VbkChainParams>(vbk_params);

  VbkBlockTree vbktree = VbkBlockTree(vbk_params, btc_params);
  BlockTree<BtcBlock, BtcChainParams>& btctree = vbktree.btc();

  std::map<BtcBlock::hash_t, std::vector<BtcTx>> btctxes;

  ValidationState state_;
};

}  // namespace altintegration

#endif
