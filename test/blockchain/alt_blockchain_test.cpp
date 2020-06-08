// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct AltTreeFixture : public ::testing::Test, public PopTestFixture {};

TEST_F(AltTreeFixture, invalidate_block_test1) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 50 blocks
  mineAltBlocks(50, chain);

  // make fork
  auto forkchain1 = chain;
  forkchain1.resize(30);
  mineAltBlocks(15, forkchain1);

  auto forkchain2 = chain;
  forkchain2.resize(15);
  mineAltBlocks(10, forkchain2);

  AltBlock endorsedBlock = chain[5];

  // generate endorsements
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  AltEndorsement endorsement1 = AltEndorsement::fromContainer(altPayloads1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  ASSERT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  auto* endorsedBlockIndex = alttree.getBlockIndex(endorsement1.endorsedHash);
  auto* containingBlockIndex1 =
      alttree.getBlockIndex(endorsement1.containingHash);
  EXPECT_TRUE(std::find(containingBlockIndex1->payloadIds.begin(),
                        containingBlockIndex1->payloadIds.end(),
                        endorsement1.parentId) !=
              containingBlockIndex1->payloadIds.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  // generate endorsements
  tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(*forkchain1.rbegin());
  forkchain1.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  AltEndorsement endorsement2 = AltEndorsement::fromContainer(altPayloads2);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  auto* containingBlockIndex2 =
      alttree.getBlockIndex(endorsement2.containingHash);

  EXPECT_TRUE(std::find(containingBlockIndex2->payloadIds.begin(),
                        containingBlockIndex2->payloadIds.end(),
                        endorsement2.parentId) !=
              containingBlockIndex2->payloadIds.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(*forkchain2.rbegin());
  forkchain2.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  AltEndorsement endorsement3 = AltEndorsement::fromContainer(altPayloads3);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  ASSERT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement3.endorsedHash);
  auto* containingBlockIndex3 =
      alttree.getBlockIndex(endorsement3.containingHash);
  EXPECT_TRUE(std::find(containingBlockIndex3->payloadIds.begin(),
                        containingBlockIndex3->payloadIds.end(),
                        endorsement3.parentId) !=
              containingBlockIndex3->payloadIds.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  // remove block
  AltBlock removeBlock = chain[20];
  alttree.removeSubtree(removeBlock.getHash());

  containingBlockIndex3 = alttree.getBlockIndex(endorsement3.containingHash);
  EXPECT_TRUE(std::find(containingBlockIndex3->payloadIds.begin(),
                        containingBlockIndex3->payloadIds.end(),
                        endorsement3.parentId) !=
              containingBlockIndex3->payloadIds.end());

  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  EXPECT_TRUE(alttree.setState(forkchain2.rbegin()->getHash(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(),
            popminer->vbk().getBestChain().tip()->getHash());
}

TEST_F(AltTreeFixture, compareTrees) {
  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam);
  EXPECT_TRUE(alttree2.bootstrap(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.vbk().btc().bootstrapWithGenesis(state));
  EXPECT_EQ(alttree, alttree2);
  EXPECT_EQ(alttree.vbk(), alttree2.vbk());
  EXPECT_EQ(alttree.vbk().btc(), alttree2.vbk().btc());

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 1 blocks
  mineAltBlocks(1, chain);
  EXPECT_NE(alttree, alttree2);
  EXPECT_EQ(alttree.vbk(), alttree2.vbk());
  EXPECT_EQ(alttree.vbk().btc(), alttree2.vbk().btc());
}

TEST_F(AltTreeFixture, validatePayloads_test) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads1 =
      generateAltPayloads(tx1,
                          containingBlock,
                          endorsedBlock1,
                          vbkparam.getGenesisBlock().getHash(),
                          0);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));

  EXPECT_TRUE(
      alttree.validatePayloads(containingBlock.getHash(), payloads1, state));
  EXPECT_TRUE(state.IsValid());

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  EXPECT_TRUE(containingIndex->isValid());
  EXPECT_EQ(*alttree.vbk().getBestChain().tip(),
            *popminer->vbk().getBestChain().tip());

  EXPECT_FALSE(
      alttree.validatePayloads(containingBlock.getHash(), payloads1, state));
  EXPECT_FALSE(state.IsValid());
  containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  EXPECT_TRUE(containingIndex->isValid());
}
