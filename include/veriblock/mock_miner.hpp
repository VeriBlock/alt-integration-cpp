#ifndef ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP
#define ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP

#include <cassert>
#include <memory>
#include <vector>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/storage/payloads_repository_inmem.hpp"

namespace altintegration {

struct Publications {
  ATV atv;
  std::vector<VTB> vtbs;
};

class MockMiner {
 public:
  using btc_block_t = BtcBlock;
  using btc_params_t = BtcChainParams;
  using btc_block_tree = BlockTree<btc_block_t, btc_params_t>;
  using btc_block_index_t = btc_block_tree::index_t;

  using vbk_block_t = VbkBlock;
  using vbk_params_t = VbkChainParams;
  using vbk_block_tree = VbkBlockTree;
  using vbk_block_index_t = vbk_block_tree::index_t;

 private:
  std::shared_ptr<BtcChainParams> btc_params =
      std::make_shared<BtcChainParamsRegTest>();
  std::shared_ptr<Miner<btc_block_t, btc_params_t>> btc_miner;
  std::shared_ptr<btc_block_tree> btc_blockchain;

  std::shared_ptr<VbkChainParams> vbk_params =
      std::make_shared<VbkChainParamsRegTest>();
  std::shared_ptr<Miner<vbk_block_t, vbk_params_t>> vbk_miner;
  std::shared_ptr<vbk_block_tree> vbk_blockchain;

  std::shared_ptr<PayloadsRepository<VTB>> vtbp_ =
      std::make_shared<PayloadsRepositoryInmem<VTB>>();

 public:
  VbkTx generateSignedVbkTx(const PublicationData& publicationData);

  VbkPopTx generateSignedVbkPoptx(const VbkBlock& publishedBlock,
                                  const BtcBlock::hash_t& lastKnownBtcBlockHash,
                                  ValidationState& state);

  ATV generateATV(const PublicationData& publicationData);

  ATV generateAndApplyATV(const PublicationData& publicationData,
                          const VbkBlock::hash_t& lastKnownVbkBlockHash,
                          ValidationState& state);

  VTB generateVTB(const VbkBlock& publishedBlock,
                  const BtcBlock::hash_t& lastKnownBtcBlockHash,
                  ValidationState& state);

  VTB generateAndApplyVTB(VbkBlockTree& tree,
                          const VbkBlock& publishedBlock,
                          const BtcBlock::hash_t& lastKnownBtcBlockHash,
                          ValidationState& state);

 public:
  MockMiner() {
    btc_miner = std::make_shared<Miner<btc_block_t, btc_params_t>>(*btc_params);
    btc_blockchain = std::make_shared<btc_block_tree>(*btc_params);

    vbk_miner = std::make_shared<Miner<vbk_block_t, vbk_params_t>>(*vbk_params);

    vbk_block_tree::PopForkComparator vbkcmp(*vtbp_, *btc_params, *vbk_params);
    vbk_blockchain =
        std::make_shared<vbk_block_tree>(*vbk_params, std::move(vbkcmp));
  }

  bool mineBtcBlocks(const uint32_t& n, ValidationState& state);
  bool mineVbkBlocks(const uint32_t& n, ValidationState& state);

  btc_block_tree& btc() { return *btc_blockchain; }
  vbk_block_tree& vbk() { return *vbk_blockchain; }

  std::shared_ptr<VbkChainParams> getVbkParams() const { return vbk_params; }
  std::shared_ptr<BtcChainParams> getBtcParams() const { return btc_params; }
};

}  // namespace altintegration

#endif
