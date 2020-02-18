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

namespace VeriBlock {

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
  using vbk_block_tree = BlockTree<vbk_block_t, vbk_params_t>;
  using vbk_block_index_t = vbk_block_tree::index_t;

  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<Miner<btc_block_t, btc_params_t>> btc_miner;
  std::shared_ptr<btc_block_tree> btc_blockchain;
  std::shared_ptr<BlockRepository<btc_block_index_t>> btc_repo;

  std::shared_ptr<VbkChainParams> vbk_params;
  std::shared_ptr<Miner<vbk_block_t, vbk_params_t>> vbk_miner;
  std::shared_ptr<vbk_block_tree> vbk_blockchain;
  std::shared_ptr<BlockRepository<vbk_block_index_t>> vbk_repo;

 private:
  ValidationState state;

  VbkTx generateSignedVbkTx(const PublicationData& publicationData);
  ATV generateValidATV(const PublicationData& publicationData,
                       const VbkBlock::hash_t& lastKnownVbkBlockHash);

  VbkPopTx generateSignedVbkPoptx(
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash);
  VTB generateValidVTB(const VbkBlock& publishedBlock,
                       const BtcBlock::hash_t& lastKnownBtcBlockHash,
                       const uint32_t& vbkBlockDelay);

 public:
  MockMiner(std::shared_ptr<BlockRepository<btc_block_index_t>> btc_repo,
            std::shared_ptr<BlockRepository<vbk_block_index_t>> vbk_repo)
      : btc_repo(btc_repo), vbk_repo(vbk_repo) {
    bool res = false;

    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btc_miner = std::make_shared<Miner<btc_block_t, btc_params_t>>(btc_params);
    btc_blockchain = std::make_shared<btc_block_tree>(btc_repo, btc_params);

    res = btc_blockchain->bootstrapWithGenesis(state);
    assert(res);

    vbk_params = std::make_shared<VbkChainParamsRegTest>();
    vbk_miner = std::make_shared<Miner<vbk_block_t, vbk_params_t>>(vbk_params);
    vbk_blockchain = std::make_shared<vbk_block_tree>(vbk_repo, vbk_params);

    res = vbk_blockchain->bootstrapWithGenesis(state);
    assert(res);
  }

  Publications mine(const PublicationData& publicationData,
                    const VbkBlock::hash_t& lastKnownVbkBlockHash,
                    const BtcBlock::hash_t& lastKnownBtcBlockHash,
                    const uint32_t& vbkBlockDelay);

  void addNewBtcBlocksIntoChainState(const uint32_t& n);
  void addNewVbkBlocksIntoChainState(const uint32_t& n);

  std::shared_ptr<VbkChainParams> getVbkParams() const { return vbk_params; }
  std::shared_ptr<BtcChainParams> getBtcParams() const { return btc_params; }
};

}  // namespace VeriBlock

#endif
