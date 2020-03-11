#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/storage/endorsement_repository_inmem.hpp"
#include "veriblock/time.hpp"

using namespace VeriBlock;

struct VbkBlockTreeTest : public VbkBlockTree {
  ~VbkBlockTreeTest() override = default;

  VbkBlockTreeTest(
      BtcTree& btc,
      std::shared_ptr<EndorsementRepository<BtcEndorsement>> endorsement_repo,
      std::shared_ptr<VbkChainParams> params)
      : VbkBlockTree(btc, std::move(endorsement_repo), std::move(params)),
        btcTree(btc),
        endorsement_repo(endorsement_repo) {}

  BtcTree& btcTree;
  std::shared_ptr<EndorsementRepository<BtcEndorsement>> endorsement_repo;
};

struct VbkBlockTreeTestFixture : ::testing::Test {
  std::shared_ptr<VbkBlockTreeTest> vbkTest;

  std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> btc_repo;
  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> btcTree;

  std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> vbk_repo;
  std::shared_ptr<VbkChainParams> vbk_params;

  std::shared_ptr<EndorsementRepository<BtcEndorsement>> endorsment_repo;

  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> btc_miner;
  std::shared_ptr<Miner<VbkBlock, VbkChainParams>> vbk_miner;

  std::shared_ptr<MockMiner> mock_miner;

  ValidationState state;

  void endorseVtbBlock(uint32_t height) {
    VTB vtb =
        mock_miner->generateValidVTB(vbkTest->getBestChain()[height]->header,
                                     vbkTest->getBestChain().tip()->getHash(),
                                     btcTree->getBestChain().tip()->getHash(),
                                     state);
    ASSERT_TRUE(state.IsValid());

    processVtb(vtb);
  }

  void processVtb(const VTB& vtb) {
    for (const auto& block : vtb.transaction.blockOfProofContext) {
      ASSERT_TRUE(btcTree->acceptBlock(block, state));
      ASSERT_TRUE(state.IsValid());
    }

    ASSERT_TRUE(btcTree->acceptBlock(vtb.transaction.blockOfProof, state));
    ASSERT_TRUE(state.IsValid());

    for (const auto& block : vtb.context) {
      ASSERT_TRUE(vbkTest->acceptBlock(block, state));
      ASSERT_TRUE(state.IsValid());
    }

    ASSERT_TRUE(vbkTest->acceptBlock(vtb.containingBlock, state));
    ASSERT_TRUE(state.IsValid());

    endorsment_repo->put(vtb);
  }

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

  VbkBlockTreeTestFixture() {
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btcTree = std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btc_params);

    vbk_params = std::make_shared<VbkChainParamsRegTest>();

    endorsment_repo =
        std::make_shared<EndorsementRepositoryInmem<BtcEndorsement>>();

    btc_miner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(btc_params);
    vbk_miner = std::make_shared<Miner<VbkBlock, VbkChainParams>>(vbk_params);

    vbkTest = std::make_shared<VbkBlockTreeTest>(
        *btcTree, endorsment_repo, vbk_params);

    mock_miner = std::make_shared<MockMiner>();

    setUpChains();
  }
};

TEST_F(VbkBlockTreeTestFixture, getProtoKeystoneContext_test) {
  uint32_t numVbkBlocks = 200;

  // in the mock_miner chain state will be 200 blocks + genesis block = 201
  // blocks
  mock_miner->mineVbkBlocks(numVbkBlocks, state);
  ASSERT_TRUE(state.IsValid());

  // initially we have to endorse the genesis block to update the vbkTest chain
  // with the 200 context blocks from the mock_miner state
  // As a result vbkTest chain state will have 200 blocks + genesis block + 1
  // block which contains the endorsement (in sum 202 blocks, and the height of
  // the chain is 201)
  endorseVtbBlock(0);

  ASSERT_TRUE((uint32_t)vbkTest->getBestChain().blocksCount() ==
              numVbkBlocks + 2);

  // endorse 176 block
  endorseVtbBlock(176);

  // endorse 166 block
  endorseVtbBlock(166);

  // endorse 169 block
  endorseVtbBlock(169);

  // endorse 143 block twice
  endorseVtbBlock(143);
  endorseVtbBlock(143);

  // endorse 87 block three times
  endorseVtbBlock(87);
  endorseVtbBlock(87);

  // endorse 91 twice
  endorseVtbBlock(91);
  endorseVtbBlock(91);

  std::vector<ProtoKeystoneContext<BtcBlock>> protoContext =
      getProtoKeystoneContext(
          vbkTest->getBestChain(), vbkTest->btcTree, vbkTest->endorsement_repo);

  EXPECT_EQ(protoContext.size(), numVbkBlocks / VBK_KEYSTONE_INTERVAL);

  EXPECT_EQ(protoContext[0].blockHeight, 20);
  EXPECT_EQ(protoContext[0].referencedByBlocks.size(), 0);

  // keystone with the height 80
  EXPECT_EQ(protoContext[3].referencedByBlocks.size(), 4);
  EXPECT_EQ(protoContext[3].blockHeight, 80);
  // keystone with the height 140
  EXPECT_EQ(protoContext[6].referencedByBlocks.size(), 2);
  EXPECT_EQ(protoContext[6].blockHeight, 140);
  // keystone with the height 160
  EXPECT_EQ(protoContext[7].referencedByBlocks.size(), 3);
  EXPECT_EQ(protoContext[7].blockHeight, 160);
}

TEST_F(VbkBlockTreeTestFixture, getKeystoneContext_test) {
  uint32_t numVbkBlocks = 200;

  // in the mock_miner chain state will be 200 blocks + genesis block = 201
  // blocks
  mock_miner->mineVbkBlocks(numVbkBlocks, state);
  ASSERT_TRUE(state.IsValid());

  // initially we have to endorse the genesis block to update the vbkTest chain
  // with the 200 context blocks from the mock_miner state
  // As a result vbkTest chain state will have 200 blocks + genesis block + 1
  // block which contains the endorsement (in sum 202 blocks, and the height of
  // the chain is 201)
  endorseVtbBlock(0);  // btc block height 1

  ASSERT_TRUE((uint32_t)vbkTest->getBestChain().blocksCount() ==
              numVbkBlocks + 2);

  // endorse 176 block
  endorseVtbBlock(176);  // btc block height 2

  // endorse 166 block
  endorseVtbBlock(166);  // btc block height 3

  // endorse 169 block
  endorseVtbBlock(169);  // btc block height 4

  // endorse 143 block twice
  endorseVtbBlock(143);  // btc block height 5
  endorseVtbBlock(143);  // btc block height 6

  // endorse 87 block three times
  endorseVtbBlock(87);  // btc block height 7
  endorseVtbBlock(87);  // btc block height 8

  // endorse 91 twice
  endorseVtbBlock(91);  // btc block height 9
  endorseVtbBlock(91);  // btc block height 10

  std::vector<KeystoneContext> keystoneContext = getKeystoneContext(
      getProtoKeystoneContext(
          vbkTest->getBestChain(), vbkTest->btcTree, vbkTest->endorsement_repo),
      vbkTest->btcTree);

  EXPECT_EQ(keystoneContext.size(), numVbkBlocks / VBK_KEYSTONE_INTERVAL);

  EXPECT_EQ(keystoneContext[0].vbkBlockHeight, 20);
  EXPECT_EQ(keystoneContext[0].firstBtcBlockPublicationHeight,
            std::numeric_limits<int32_t>::max());

  // keystone with the height 80
  EXPECT_EQ(keystoneContext[3].vbkBlockHeight, 80);
  EXPECT_EQ(keystoneContext[3].firstBtcBlockPublicationHeight, 7);
  // keystone with the height 140
  EXPECT_EQ(keystoneContext[6].vbkBlockHeight, 140);
  EXPECT_EQ(keystoneContext[6].firstBtcBlockPublicationHeight, 5);
  // keystone with the height 160
  EXPECT_EQ(keystoneContext[7].vbkBlockHeight, 160);
  EXPECT_EQ(keystoneContext[7].firstBtcBlockPublicationHeight, 2);
}
