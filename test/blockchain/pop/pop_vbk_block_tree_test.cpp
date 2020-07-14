// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <exception>
#include <memory>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"

using namespace altintegration;

struct VbkBlockTreeTestFixture : public ::testing::Test {
  MockMiner popminer;

  void endorseVBK(size_t height) {
    auto* endorsedIndex = popminer.vbk().getBestChain()[(int32_t)height];
    ASSERT_TRUE(endorsedIndex);
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedIndex->getHeader());
    auto btccontaining = popminer.mineBtcBlocks(1);
    auto vbkpoptx = popminer.createVbkPopTxEndorsingVbkBlock(
        btccontaining->getHeader(),
        btctx,
        endorsedIndex->getHeader(),
        popminer.getBtcParams().getGenesisBlock().getHash());
    popminer.mineVbkBlocks(1);
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto Btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer.mineBtcBlocks(1);
    return popminer.createVbkPopTxEndorsingVbkBlock(
        btcBlockTip->getHeader(),
        Btctx,
        endorsedBlock,
        popminer.getBtcParams().getGenesisBlock().getHash());
  }
};

TEST_F(VbkBlockTreeTestFixture, FilterChainForForkResolution) {
  using namespace internal;

  size_t numVbkBlocks = 200ull;
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

  auto max = (std::numeric_limits<int32_t>::max)();
  EXPECT_EQ(keystoneContext[0].blockHeight, 20);
  EXPECT_EQ(keystoneContext[0].firstBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[1].blockHeight, 40);
  EXPECT_EQ(keystoneContext[1].firstBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[2].blockHeight, 60);
  EXPECT_EQ(keystoneContext[2].firstBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[3].blockHeight, 80);
  EXPECT_EQ(keystoneContext[3].firstBlockPublicationHeight, 6);
  EXPECT_EQ(keystoneContext[4].blockHeight, 100);
  EXPECT_EQ(keystoneContext[4].firstBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[5].blockHeight, 120);
  EXPECT_EQ(keystoneContext[5].firstBlockPublicationHeight, max);
  EXPECT_EQ(keystoneContext[6].blockHeight, 140);
  EXPECT_EQ(keystoneContext[6].firstBlockPublicationHeight, 4);
  EXPECT_EQ(keystoneContext[7].blockHeight, 160);
  EXPECT_EQ(keystoneContext[7].firstBlockPublicationHeight, 1);
}

TEST_F(VbkBlockTreeTestFixture, addAllPayloads_failure_test) {
  // start with 30 BTC blocks
  auto* btcBlockTip = popminer.mineBtcBlocks(30);

  ASSERT_EQ(btcBlockTip->getHash(),
            popminer.btc().getBestChain().tip()->getHash());

  // start with 65 VBK blocks
  auto* vbkBlockTip = popminer.mineVbkBlocks(65);

  ASSERT_EQ(popminer.vbk().getBestChain().tip()->getHash(),
            vbkBlockTip->getHash());

  // Make 5 endorsements valid endorsements
  auto* endorsedVbkBlock1 =
      vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 11);
  ASSERT_EQ(endorsedVbkBlock1->endorsedBy.size(), 0);
  auto* endorsedVbkBlock2 =
      vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 12);
  ASSERT_EQ(endorsedVbkBlock2->endorsedBy.size(), 0);
  auto* endorsedVbkBlock3 =
      vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 13);
  ASSERT_EQ(endorsedVbkBlock3->endorsedBy.size(), 0);
  auto* endorsedVbkBlock4 =
      vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 14);
  ASSERT_EQ(endorsedVbkBlock4->endorsedBy.size(), 0);
  auto* endorsedVbkBlock5 =
      vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 15);
  ASSERT_EQ(endorsedVbkBlock5->endorsedBy.size(), 0);

  generatePopTx(endorsedVbkBlock1->getHeader());
  generatePopTx(endorsedVbkBlock2->getHeader());
  generatePopTx(endorsedVbkBlock3->getHeader());
  generatePopTx(endorsedVbkBlock4->getHeader());
  generatePopTx(endorsedVbkBlock5->getHeader());
  ASSERT_EQ(popminer.vbkmempool.size(), 5);

  vbkBlockTip = popminer.mineVbkBlocks(1);

  ASSERT_EQ(popminer.vbk().getBestChain().tip()->getHash(),
            vbkBlockTip->getHash());

  // check that we have endorsements to the VbBlocks
  ASSERT_EQ(endorsedVbkBlock1->endorsedBy.size(), 1);
  ASSERT_EQ(endorsedVbkBlock2->endorsedBy.size(), 1);
  ASSERT_EQ(endorsedVbkBlock3->endorsedBy.size(), 1);
  ASSERT_EQ(endorsedVbkBlock4->endorsedBy.size(), 1);
  ASSERT_EQ(endorsedVbkBlock5->endorsedBy.size(), 1);

  // mine 40 Vbk blocks
  vbkBlockTip = popminer.mineVbkBlocks(40);
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->getHash(),
            vbkBlockTip->getHash());

  // Make 5 endorsements valid endorsements
  endorsedVbkBlock1 = vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 11);
  ASSERT_EQ(endorsedVbkBlock1->endorsedBy.size(), 0);
  endorsedVbkBlock2 = vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 12);
  ASSERT_EQ(endorsedVbkBlock2->endorsedBy.size(), 0);
  endorsedVbkBlock3 = vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 13);
  ASSERT_EQ(endorsedVbkBlock3->endorsedBy.size(), 0);
  endorsedVbkBlock4 = vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 14);
  ASSERT_EQ(endorsedVbkBlock4->endorsedBy.size(), 0);
  endorsedVbkBlock5 = vbkBlockTip->getAncestor(vbkBlockTip->getHeight() - 15);
  ASSERT_EQ(endorsedVbkBlock5->endorsedBy.size(), 0);

  generatePopTx(endorsedVbkBlock1->getHeader());
  generatePopTx(endorsedVbkBlock2->getHeader());
  generatePopTx(endorsedVbkBlock3->getHeader());
  generatePopTx(endorsedVbkBlock4->getHeader());
  generatePopTx(endorsedVbkBlock5->getHeader());
  ASSERT_EQ(popminer.vbkmempool.size(), 5);

  // corrupt one of the endorsement
  std::vector<uint8_t> new_hash = {1, 2, 3};
  popminer.vbkmempool[0].blockOfProof._previousBlock = uint256(new_hash);

  EXPECT_THROW(popminer.mineVbkBlocks(1), std::domain_error);

  // check that all endorsement have not been applied
  ASSERT_EQ(endorsedVbkBlock1->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock2->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock3->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock4->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock5->endorsedBy.size(), 0);
}
