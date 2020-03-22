#include <gtest/gtest.h>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/rewards/poprewards.hpp"

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getGenesisBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

struct RewardsTestFixture : ::testing::Test {
  std::shared_ptr<VbkBlockTree> vbkTest;

  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> btcTree;

  std::shared_ptr<VbkChainParams> vbk_params;

  std::shared_ptr<EndorsementRepository<BtcEndorsement>> endorsement_repo;

  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> btc_miner;
  std::shared_ptr<Miner<VbkBlock, VbkChainParams>> vbk_miner;

  std::shared_ptr<MockMiner> mock_miner;

  ValidationState state;

  void setUpChains() {
    ASSERT_TRUE(mock_miner->btc().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(mock_miner->vbk().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());

    ASSERT_TRUE(btcTree->bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(vbkTest->bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
  }

  RewardsTestFixture() {
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btcTree = std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btc_params);

    vbk_params = std::make_shared<VbkChainParamsRegTest>();

    endorsement_repo =
        std::make_shared<EndorsementRepositoryInmem<BtcEndorsement>>();

    btc_miner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(btc_params);
    vbk_miner = std::make_shared<Miner<VbkBlock, VbkChainParams>>(vbk_params);

    vbkTest =
        std::make_shared<VbkBlockTree>(*btcTree, endorsement_repo, vbk_params);

    mock_miner = std::make_shared<MockMiner>();

    setUpChains();
  }
};

TEST_F(RewardsTestFixture, basicReward_test) {
}
