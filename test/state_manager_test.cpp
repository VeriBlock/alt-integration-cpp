#include <gtest/gtest.h>

#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/state_manager.hpp"
#include "veriblock/state_utils.hpp"
#include "veriblock/storage/repository_rocks_manager.hpp"

using namespace VeriBlock;

// DB name
static const std::string dbName = "db-test";
static const uint32_t blocksNumber = 1000;

struct TestFixture : public ::testing::Test {
  StateManager<RepositoryRocksManager> stateManager;

  std::shared_ptr<BtcChainParams> btcParams;
  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> expectedBtcBlockTree;
  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> btcMiner;

  std::shared_ptr<VbkChainParams> vbkParams;
  std::shared_ptr<BlockTree<VbkBlock, VbkChainParams>> expectedVbkBlockTree;
  std::shared_ptr<Miner<VbkBlock, VbkChainParams>> vbkMiner;

  ValidationState state;

  void uploadBlocksToTreeAndRepo() {
    BlockIndex<BtcBlock> temp_index_btc;
    for (uint32_t i = 0; i < blocksNumber; ++i) {
      BtcBlock newBtcBlock = btcMiner->createNextBlock(
          *expectedBtcBlockTree->getBestChain().tip(), {});

      ASSERT_TRUE(expectedBtcBlockTree->acceptBlock(
          newBtcBlock, state, &temp_index_btc));
      ASSERT_TRUE(state.IsValid());
      stateManager.putBtcBlock(temp_index_btc);
    }

    BlockIndex<VbkBlock> temp_index_vbk;
    for (uint32_t i = 0; i < blocksNumber; ++i) {
      VbkBlock newVbkBlock = vbkMiner->createNextBlock(
          *expectedVbkBlockTree->getBestChain().tip(), {});

      ASSERT_TRUE(expectedVbkBlockTree->acceptBlock(
          newVbkBlock, state, &temp_index_vbk));
      ASSERT_TRUE(state.IsValid());
      stateManager.putVbkBlock(temp_index_vbk);
    }
    // stateManager.commit();
  }

  TestFixture() : stateManager(dbName) {
    stateManager.wipeRepos();

    btcParams = std::make_shared<BtcChainParamsRegTest>();
    expectedBtcBlockTree =
        std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btcParams);
    btcMiner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(btcParams);

    vbkParams = std::make_shared<VbkChainParamsRegTest>();
    expectedVbkBlockTree =
        std::make_shared<BlockTree<VbkBlock, VbkChainParams>>(vbkParams);
    vbkMiner = std::make_shared<Miner<VbkBlock, VbkChainParams>>(vbkParams);

    // @when
    EXPECT_TRUE(expectedBtcBlockTree->bootstrapWithGenesis(state))
        << "bootstrapWithGenesis: " << state.GetRejectReason() << ", "
        << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());

    // @when
    EXPECT_TRUE(expectedVbkBlockTree->bootstrapWithGenesis(state))
        << "bootstrapWithGenesis: " << state.GetRejectReason() << ", "
        << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());

    uploadBlocksToTreeAndRepo();
  }
};

TEST_F(TestFixture, LoadBlockTree_test) {
  BlockTree<BtcBlock, BtcChainParams> btcTree(btcParams);
  BlockTree<VbkBlock, VbkChainParams> vbkTree(vbkParams);

  EXPECT_TRUE(btcTree.bootstrapWithGenesis(state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_TRUE(vbkTree.bootstrapWithGenesis(state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_TRUE(loadBlockTree(btcTree, stateManager.getBtcCursor(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(btcTree.getBestChain().tip()->header,
            expectedBtcBlockTree->getBestChain().tip()->header);

  EXPECT_EQ(btcTree.getBestChain().tip()->getHash(),
            expectedBtcBlockTree->getBestChain().tip()->getHash());

  while (btcTree.getBestChain().blocksCount() > 1) {
    btcTree.invalidateTip();
    expectedBtcBlockTree->invalidateTip();

    EXPECT_EQ(btcTree.getBestChain().tip()->header,
              expectedBtcBlockTree->getBestChain().tip()->header);

    EXPECT_EQ(btcTree.getBestChain().tip()->getHash(),
              expectedBtcBlockTree->getBestChain().tip()->getHash());
  }

  EXPECT_TRUE(loadBlockTree(vbkTree, stateManager.getVbkCursor(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(vbkTree.getBestChain().tip()->header,
            expectedVbkBlockTree->getBestChain().tip()->header);

  EXPECT_EQ(vbkTree.getBestChain().tip()->getHash(),
            expectedVbkBlockTree->getBestChain().tip()->getHash());

  while (vbkTree.getBestChain().blocksCount() > 1) {
    vbkTree.invalidateTip();
    expectedVbkBlockTree->invalidateTip();

    EXPECT_EQ(vbkTree.getBestChain().tip()->header,
              expectedVbkBlockTree->getBestChain().tip()->header);

    EXPECT_EQ(vbkTree.getBestChain().tip()->getHash(),
              expectedVbkBlockTree->getBestChain().tip()->getHash());
  }
}
