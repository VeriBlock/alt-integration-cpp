#include <gtest/gtest.h>

#include <memory>

#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/storage/endorsements_repository_inmem.hpp"
#include "veriblock/time.hpp"

using namespace VeriBlock;

struct VbkBlockTreeTest : public VbkBlockTree {
  ~VbkBlockTreeTest() override = default;

  VbkBlockTreeTest(
      BtcTree& btc,
      std::shared_ptr<EndorsementsRepository> endorsment_repo,
      std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> blockrepo,
      std::shared_ptr<VbkChainParams> params)
      : VbkBlockTree(btc, endorsment_repo, blockrepo, params) {}

  std::vector<ProtoKeystoneContext> getProtoKeystoneContextTest() {
    return getProtoKeystoneContext(this->getBestChain());
  }

  std::vector<KeystoneContext> getKeystoneContextTest() {
    return getKeystoneContext(getProtoKeystoneContext(this->getBestChain()));
  }
};

struct VbkBlockTreeTestFixture : ::testing::Test {
  std::shared_ptr<VbkBlockTreeTest> vbkTest;

  std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> btc_repo;
  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> btcTree;

  std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> vbk_repo;
  std::shared_ptr<VbkChainParams> vbk_params;

  std::shared_ptr<EndorsementsRepository> endorsment_repo;

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

    endorsment_repo->put(vtb.transaction, vtb.containingBlock.getHash());
  }

  void setUpChains() {
    ASSERT_TRUE(mock_miner->bootstrapBtcChainWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(mock_miner->bootstrapVbkChainWithGenesis(state));
    ASSERT_TRUE(state.IsValid());

    ASSERT_TRUE(btcTree->bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(vbkTest->bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
  }

  VbkBlockTreeTestFixture() {
    btc_repo = std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btcTree = std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btc_repo,
                                                                    btc_params);

    vbk_repo = std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    vbk_params = std::make_shared<VbkChainParamsRegTest>();

    endorsment_repo = std::make_shared<EndorsementsRepositoryInmem>();

    btc_miner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(
        btc_params, currentTimestamp4());
    vbk_miner = std::make_shared<Miner<VbkBlock, VbkChainParams>>(
        vbk_params, currentTimestamp4());

    vbkTest = std::make_shared<VbkBlockTreeTest>(
        *btcTree, endorsment_repo, vbk_repo, vbk_params);

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

  ASSERT_TRUE((uint32_t)vbkTest->getBestChain().height() == numVbkBlocks + 1);

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

  std::vector<ProtoKeystoneContext> protoContext =
      vbkTest->getProtoKeystoneContextTest();

  EXPECT_EQ(protoContext.size(), numVbkBlocks / VBK_KEYSTONE_INTERVAL);

  EXPECT_EQ(protoContext[0].vbkBlockHeight, 20);
  EXPECT_EQ(protoContext[0].referencedByBtcBlocks.size(), 0);

  // keystone with the height 80
  EXPECT_EQ(protoContext[3].referencedByBtcBlocks.size(), 4);
  EXPECT_EQ(protoContext[3].vbkBlockHeight, 80);
  // keystone with the height 140
  EXPECT_EQ(protoContext[6].referencedByBtcBlocks.size(), 2);
  EXPECT_EQ(protoContext[6].vbkBlockHeight, 140);
  // keystone with the height 160
  EXPECT_EQ(protoContext[7].referencedByBtcBlocks.size(), 3);
  EXPECT_EQ(protoContext[7].vbkBlockHeight, 160);
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

  ASSERT_TRUE((uint32_t)vbkTest->getBestChain().height() == numVbkBlocks + 1);

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

  std::vector<KeystoneContext> keystoneContext =
      vbkTest->getKeystoneContextTest();

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
