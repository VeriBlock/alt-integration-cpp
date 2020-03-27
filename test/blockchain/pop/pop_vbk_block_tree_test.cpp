#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "util/test_blockchain_fixture.hpp"
#include "util/visualize.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/storage/endorsement_repository_inmem.hpp"
#include "veriblock/time.hpp"

using namespace altintegration;

struct VbkBlockTreeTestFixture : ::testing::Test {
  std::shared_ptr<VbkBlockTree> vbkTree;
  std::shared_ptr<BtcChainParams> btcp;
  std::shared_ptr<VbkChainParams> vbkp;

  std::shared_ptr<PayloadsRepository<VTB>> vtbp_ =
      std::make_shared<PayloadsRepositoryInmem<VTB>>();

  std::shared_ptr<Miner<VbkBlock, VbkChainParams>> vbkMiner;

  std::shared_ptr<MockMiner> mock_miner;

  ValidationState state;

  void endorseVBKblock(uint32_t height) {
    auto& best = vbkTree->getBestChain();
    auto publishedBlock = best[height];
    ASSERT_TRUE(publishedBlock);

    VTB vtb = mock_miner->generateAndApplyVTB(
        *vbkTree, publishedBlock->header, state);

    ASSERT_TRUE(state.IsValid()) << state.GetDebugMessage();

    vtbp_->put(vtb);
  }

  VbkBlockTreeTestFixture() {
    btcp = std::make_shared<BtcChainParamsRegTest>();
    vbkp = std::make_shared<VbkChainParamsRegTest>();

    vbkMiner = std::make_shared<Miner<VbkBlock, VbkChainParams>>(*vbkp);

    VbkBlockTree::PopForkComparator cmp(*vtbp_, *btcp, *vbkp);
    vbkTree = std::make_shared<VbkBlockTree>(*vbkp, std::move(cmp));

    mock_miner = std::make_shared<MockMiner>();

    setUpChains();
  }

  void setUpChains() {
    ASSERT_TRUE(mock_miner->btc().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());

    ASSERT_TRUE(vbkTree->btc().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(vbkTree->bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
  }
};

TEST_F(VbkBlockTreeTestFixture, getProtoKeystoneContext_test) {
  using namespace internal;

  ASSERT_EQ(vbkTree->getComparator().getIndex(), vbkTree->getBestChain().tip());

  uint32_t numVbkBlocks = 200;
  auto& best = vbkTree->getBestChain();
  for (uint32_t i = 0; i < numVbkBlocks; i++) {
    auto block = vbkMiner->createNextBlock(*best.tip());
    ASSERT_TRUE(vbkTree->acceptBlock(block, {}, state));
    ASSERT_EQ(vbkTree->getComparator().getIndex(),
              vbkTree->getBestChain().tip())
        << "iteration " << i;
  }

  // initially we have to endorse the genesis block to update the vbkTest chain
  // with the 200 context blocks from the mock_miner state
  // As a result vbkTest chain state will have 200 blocks + genesis block + 1
  // block which contains the endorsement (in sum 202 blocks, and the height of
  // the chain is 201)
  endorseVBKblock(0);
  ASSERT_EQ(best.tip()->containingEndorsements.size(), 1);

  ASSERT_TRUE((uint32_t)vbkTree->getBestChain().blocksCount() ==
              numVbkBlocks + 2);

  // endorse 176 block
  endorseVBKblock(176);

  // endorse 166 block
  endorseVBKblock(166);

  // endorse 169 block
  endorseVBKblock(169);

  // endorse 143 block twice
  endorseVBKblock(143);
  endorseVBKblock(143);

  // endorse 87 block three times
  endorseVBKblock(87);
  endorseVBKblock(87);

  // endorse 91 twice
  endorseVBKblock(91);
  endorseVBKblock(91);

  auto protoContext =
      getProtoKeystoneContext(vbkTree->getBestChain(), vbkTree->btc(), *vbkp);

  EXPECT_EQ(protoContext.size(), numVbkBlocks / vbkp->getKeystoneInterval());

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
  using namespace internal;
  uint32_t numVbkBlocks = 200;

  ASSERT_EQ(vbkTree->getComparator().getIndex(), vbkTree->getBestChain().tip());

  auto& best = vbkTree->getBestChain();
  for (uint32_t i = 0; i < numVbkBlocks; i++) {
    auto block = vbkMiner->createNextBlock(*best.tip());
    ASSERT_TRUE(vbkTree->acceptBlock(block, {}, state));
    ASSERT_EQ(vbkTree->getComparator().getIndex(),
              vbkTree->getBestChain().tip())
        << "iteration " << i;
  }

  // initially we have to endorse the genesis block to update the vbkTest chain
  // with the 200 context blocks from the mock_miner state
  // As a result vbkTest chain state will have 200 blocks + genesis block +
  // 1 block which contains the endorsement (in sum 202 blocks, and the height
  // of the chain is 201)
  endorseVBKblock(0);  // btc block height 1

  ASSERT_TRUE((uint32_t)vbkTree->getBestChain().blocksCount() ==
              numVbkBlocks + 2);

  // endorse 176 block
  endorseVBKblock(176);  // btc block height 2

  // endorse 166 block
  endorseVBKblock(166);  // btc block height 3

  // endorse 169 block
  endorseVBKblock(169);  // btc block height 4

  // endorse 143 block twice
  endorseVBKblock(143);  // btc block height 5
  endorseVBKblock(143);  // btc block height 6

  // endorse 87 block three times
  endorseVBKblock(87);  // btc block height 7
  endorseVBKblock(87);  // btc block height 8

  // endorse 91 twice
  endorseVBKblock(91);  // btc block height 9
  endorseVBKblock(91);  // btc block height 10

  auto pkc = getProtoKeystoneContext(
      vbkTree->getBestChain(), vbkTree->btc(), *this->vbkp);
  auto keystoneContext = getKeystoneContext(pkc, vbkTree->btc());

  EXPECT_EQ(keystoneContext.size(),
            numVbkBlocks / this->vbkp->getKeystoneInterval());

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
