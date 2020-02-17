#ifndef ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP
#define ALT_INTEGRATION_VERIBLOCK_MOCK_MINER_HPP

#include <stdint.h>

#include <cassert>
#include <memory>
#include <vector>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

namespace VeriBlock {

struct Publications {
  ATV atv;
  std::vector<VTB> vtbs;
};

class MockMiner {
  using btc_block_t = BtcBlock;
  using btc_params_t = BtcChainParams;
  using btc_block_tree = BlockTree<btc_block_t, btc_params_t>;
  using btc_block_index_t = btc_block_tree::index_t;

  using vbk_block_t = VbkBlock;
  using vbk_params_t = VbkChainParams;
  using vbk_block_tree = BlockTree<vbk_block_t, vbk_params_t>;
  using vbk_block_index_t = vbk_block_tree::index_t;

  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<Miner<btc_block_t, btc_params_t>> btc_miner;
  std::shared_ptr<btc_block_tree> btc_blockchain;
  std::shared_ptr<BlockRepositoryInmem<btc_block_index_t>> btc_repo;

  std::shared_ptr<VbkChainParams> vbk_params;
  std::shared_ptr<Miner<vbk_block_t, vbk_params_t>> vbk_miner;
  std::shared_ptr<vbk_block_tree> vbk_blockchain;
  std::shared_ptr<BlockRepositoryInmem<vbk_block_index_t>> vbk_repo;

  ValidationState state;

  VbkTx generateSignedVbkTx(const PublicationData& publicationData);
  ATV generateValidATV(const PublicationData& publicationData);

  VbkPopTx generateSignedVbkPoptx(const VbkBlock& publishedBlock);
  VTB generateValidVTB(const VbkBlock& publishedBlock);

 public:
  MockMiner() {
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btc_repo = std::make_shared<BlockRepositoryInmem<btc_block_index_t>>();
    btc_miner = std::make_shared<Miner<btc_block_t, btc_params_t>>(btc_params);
    btc_blockchain = std::make_shared<btc_block_tree>(btc_repo, btc_params);
    assert(btc_blockchain->bootstrap(0, btc_params->getGenesisBlock(), state));

    vbk_params = std::make_shared<VbkChainParamsRegTest>();
    vbk_repo = std::make_shared<BlockRepositoryInmem<vbk_block_index_t>>();
    vbk_miner = std::make_shared<Miner<vbk_block_t, vbk_params_t>>(vbk_params);
    vbk_blockchain = std::make_shared<vbk_block_tree>(vbk_repo, vbk_params);
    assert(vbk_blockchain->bootstrap(vbk_params->getGenesisBlock().height,
                                     vbk_params->getGenesisBlock(),
                                     state));
  }

  Publications mine(const PublicationData& publicationData);

  std::shared_ptr<VbkChainParams> getVbkParams() const { return vbk_params; }
  std::shared_ptr<BtcChainParams> getBtcParams() const { return btc_params; }
};

}  // namespace VeriBlock

#endif
