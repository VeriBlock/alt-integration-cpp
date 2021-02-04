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
    auto btctx =
        popminer.createBtcTxEndorsingVbkBlock(endorsedIndex->getHeader());
    auto btccontaining = popminer.mineBtcBlocks(1, {btctx});
    auto vbkpoptx = popminer.createVbkPopTxEndorsingVbkBlock(
        btccontaining->getHeader(),
        btctx,
        endorsedIndex->getHeader(),
        GetRegTestBtcBlock().getHash());
    popminer.mineVbkBlocks(1, {vbkpoptx});
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcblock = popminer.mineBtcBlocks(1, {btctx});
    return popminer.createVbkPopTxEndorsingVbkBlock(
        btcblock->getHeader(),
        btctx,
        endorsedBlock,
        GetRegTestBtcBlock().getHash());
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

  auto hashes = best.getAllHashesInChain();

  auto protoContext = getProtoKeystoneContext(
      20, best, hashes, popminer.btc(), popminer.getVbkParams());

  EXPECT_EQ(protoContext.blockHeight, 20);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 0);

  protoContext = getProtoKeystoneContext(
      40, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 40);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 0);

  protoContext = getProtoKeystoneContext(
      60, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 60);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 0);

  protoContext = getProtoKeystoneContext(
      80, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 80);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 4);

  protoContext = getProtoKeystoneContext(
      100, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 100);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 0);

  protoContext = getProtoKeystoneContext(
      120, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 120);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 0);

  protoContext = getProtoKeystoneContext(
      140, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 140);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 2);

  protoContext = getProtoKeystoneContext(
      160, best, hashes, popminer.btc(), popminer.getVbkParams());
  EXPECT_EQ(protoContext.blockHeight, 160);
  EXPECT_EQ(protoContext.referencedByBlocks.size(), 3);

  internal::ReducedPublicationView reducedPublicationView{
      best, popminer.getVbkParams(), popminer.btc()};

  EXPECT_EQ(reducedPublicationView.size(),
            numVbkBlocks / popminer.getVbkParams().getKeystoneInterval());

  auto max = internal::NO_ENDORSEMENT;

  EXPECT_EQ(reducedPublicationView[20]->blockHeight, 20);
  EXPECT_EQ(reducedPublicationView[20]->firstBlockPublicationHeight, max);
  EXPECT_EQ(reducedPublicationView[40]->blockHeight, 40);
  EXPECT_EQ(reducedPublicationView[40]->firstBlockPublicationHeight, max);
  EXPECT_EQ(reducedPublicationView[60]->blockHeight, 60);
  EXPECT_EQ(reducedPublicationView[60]->firstBlockPublicationHeight, max);
  EXPECT_EQ(reducedPublicationView[80]->blockHeight, 80);
  EXPECT_EQ(reducedPublicationView[80]->firstBlockPublicationHeight, 6);
  EXPECT_EQ(reducedPublicationView[100]->blockHeight, 100);
  EXPECT_EQ(reducedPublicationView[100]->firstBlockPublicationHeight, max);
  EXPECT_EQ(reducedPublicationView[120]->blockHeight, 120);
  EXPECT_EQ(reducedPublicationView[120]->firstBlockPublicationHeight, max);
  EXPECT_EQ(reducedPublicationView[140]->blockHeight, 140);
  EXPECT_EQ(reducedPublicationView[140]->firstBlockPublicationHeight, 4);
  EXPECT_EQ(reducedPublicationView[160]->blockHeight, 160);
  EXPECT_EQ(reducedPublicationView[160]->firstBlockPublicationHeight, 1);
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

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());
  auto vbkPopTx3 = generatePopTx(endorsedVbkBlock3->getHeader());
  auto vbkPopTx4 = generatePopTx(endorsedVbkBlock4->getHeader());
  auto vbkPopTx5 = generatePopTx(endorsedVbkBlock5->getHeader());

  vbkBlockTip = popminer.mineVbkBlocks(
      1, {vbkPopTx1, vbkPopTx2, vbkPopTx3, vbkPopTx4, vbkPopTx5});

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

  vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());
  vbkPopTx3 = generatePopTx(endorsedVbkBlock3->getHeader());
  vbkPopTx4 = generatePopTx(endorsedVbkBlock4->getHeader());
  vbkPopTx5 = generatePopTx(endorsedVbkBlock5->getHeader());

  // corrupt one of the endorsement
  std::vector<uint8_t> new_hash = {1, 2, 3};
  ASSERT_GT(vbkPopTx1.blockOfProofContext.size(), 0);
  vbkPopTx1.blockOfProofContext[0].previousBlock = uint256(new_hash);

  EXPECT_THROW(popminer.mineVbkBlocks(
                   1, {vbkPopTx1, vbkPopTx2, vbkPopTx3, vbkPopTx4, vbkPopTx5}),
               std::domain_error);

  // check that all endorsement have not been applied
  ASSERT_EQ(endorsedVbkBlock1->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock2->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock3->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock4->endorsedBy.size(), 0);
  ASSERT_EQ(endorsedVbkBlock5->endorsedBy.size(), 0);
}
