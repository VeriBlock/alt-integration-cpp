#include <gtest/gtest.h>

#include <memory>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"

using namespace altintegration;

struct VbkBlockTreeTestFixture : public ::testing::Test {
  MockMiner popminer;

  void endorseVBK(size_t height) {
    auto* endorsedIndex = popminer.vbk().getBestChain()[height];
    ASSERT_TRUE(endorsedIndex);
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedIndex->header);
    auto btccontaining = popminer.mineBtcBlocks(1);
    auto vbkpoptx = popminer.createVbkPopTxEndorsingVbkBlock(
        btccontaining->header,
        btctx,
        endorsedIndex->header,
        popminer.btc().getBestChain().tip()->getHash());
    popminer.mineVbkBlocks(1);
  }
};

TEST_F(VbkBlockTreeTestFixture, FilterChainForForkResolution) {
  using namespace internal;

  size_t numVbkBlocks = 200ull;
  ASSERT_EQ(popminer.vbk().getComparator().getIndex(),
            popminer.vbk().getBestChain().tip());
  popminer.mineVbkBlocks(numVbkBlocks);

  auto& best = popminer.vbk().getBestChain();
  ASSERT_EQ(best.blocksCount(), numVbkBlocks + 1);

  endorseVBK(176);
  endorseVBK(166);
  endorseVBK(169);
  endorseVBK(143);
  endorseVBK(143);
  endorseVBK(87);
  endorseVBK(87);
  endorseVBK(91);
  endorseVBK(91);
  ASSERT_EQ(best.blocksCount(), numVbkBlocks + 10);
  popminer.mineVbkBlocks(1);
  ASSERT_EQ(best.blocksCount(), numVbkBlocks + 11);

  auto protoContext =
      getProtoKeystoneContext(best, popminer.btc(), popminer.getVbkParams());

  EXPECT_EQ(protoContext.size(),
            numVbkBlocks / popminer.getVbkParams().getKeystoneInterval());

  EXPECT_EQ(protoContext[0].blockHeight, 20);
  EXPECT_EQ(protoContext[0].referencedByBlocks.size(), 0);
  EXPECT_EQ(protoContext[1].blockHeight, 40);
  EXPECT_EQ(protoContext[1].referencedByBlocks.size(), 0);
  EXPECT_EQ(protoContext[2].blockHeight, 60);
  EXPECT_EQ(protoContext[2].referencedByBlocks.size(), 0);
  EXPECT_EQ(protoContext[3].blockHeight, 80);
  EXPECT_EQ(protoContext[3].referencedByBlocks.size(), 4);
  EXPECT_EQ(protoContext[4].blockHeight, 100);
  EXPECT_EQ(protoContext[4].referencedByBlocks.size(), 0);
  EXPECT_EQ(protoContext[5].blockHeight, 120);
  EXPECT_EQ(protoContext[5].referencedByBlocks.size(), 0);
  EXPECT_EQ(protoContext[6].blockHeight, 140);
  EXPECT_EQ(protoContext[6].referencedByBlocks.size(), 2);
  EXPECT_EQ(protoContext[7].blockHeight, 160);
  EXPECT_EQ(protoContext[7].referencedByBlocks.size(), 3);

  auto keystoneContext = getKeystoneContext(protoContext, popminer.btc());
  EXPECT_EQ(keystoneContext.size(),
            numVbkBlocks / popminer.getVbkParams().getKeystoneInterval());

  auto max = std::numeric_limits<int32_t>::max();
  EXPECT_EQ(keystoneContext[0].vbkBlockHeight, 20);
  EXPECT_EQ(keystoneContext[0].firstBtcBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[1].vbkBlockHeight, 40);
  EXPECT_EQ(keystoneContext[1].firstBtcBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[2].vbkBlockHeight, 60);
  EXPECT_EQ(keystoneContext[2].firstBtcBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[3].vbkBlockHeight, 80);
  EXPECT_EQ(keystoneContext[3].firstBtcBlockPublicationHeight, 6);
  EXPECT_EQ(keystoneContext[4].vbkBlockHeight, 100);
  EXPECT_EQ(keystoneContext[4].firstBtcBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[5].vbkBlockHeight, 120);
  EXPECT_EQ(keystoneContext[5].firstBtcBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[6].vbkBlockHeight, 140);
  EXPECT_EQ(keystoneContext[6].firstBtcBlockPublicationHeight, 4);
  EXPECT_EQ(keystoneContext[7].vbkBlockHeight, 160);
  EXPECT_EQ(keystoneContext[7].firstBtcBlockPublicationHeight, 1);
}
