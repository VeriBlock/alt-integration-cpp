// Copyright (c) 2019-2021 Xenios SEZC
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
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);
  PopData altPayloads1 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  AltEndorsement endorsement1 = AltEndorsement::fromContainer(
      altPayloads1.atvs[0], containingBlock.getHash(), endorsedBlock.getHash());

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads1));
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  verifyEndorsementAdded(alttree, endorsement1);

  // check endorsements
  auto* endorsedBlockIndex = alttree.getBlockIndex(endorsement1.endorsedHash);
  auto* containingBlockIndex1 =
      alttree.getBlockIndex(endorsement1.containingHash);
  auto& containingEndorsements =
      containingBlockIndex1->getContainingEndorsements();
  EXPECT_TRUE(containingEndorsements.find(endorsement1.id) !=
              containingEndorsements.end());
  EXPECT_EQ(endorsedBlockIndex->getEndorsedBy().size(), 1);

  // generate endorsements
  tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(forkchain1.back());
  forkchain1.push_back(containingBlock);
  PopData altPayloads2 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  AltEndorsement endorsement2 = AltEndorsement::fromContainer(
      altPayloads2.atvs[0], containingBlock.getHash(), endorsedBlock.getHash());

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), {altPayloads2}))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  verifyEndorsementAdded(alttree, endorsement2);

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  auto* containingBlockIndex2 =
      alttree.getBlockIndex(endorsement2.containingHash);
  auto& containingEndorsements2 =
      containingBlockIndex2->getContainingEndorsements();
  EXPECT_TRUE(containingEndorsements2.find(endorsement2.id) !=
              containingEndorsements2.end());
  EXPECT_EQ(endorsedBlockIndex->getEndorsedBy().size(), 1);

  tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(forkchain2.back());
  forkchain2.push_back(containingBlock);
  PopData altPayloads3 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  AltEndorsement endorsement3 = AltEndorsement::fromContainer(
      altPayloads3.atvs[0], containingBlock.getHash(), endorsedBlock.getHash());

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads3));
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  verifyEndorsementAdded(alttree, endorsement3);

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement3.endorsedHash);
  auto* containingBlockIndex3 =
      alttree.getBlockIndex(endorsement3.containingHash);
  auto& containingEndorsements3 =
      containingBlockIndex3->getContainingEndorsements();
  EXPECT_TRUE(containingEndorsements3.find(endorsement3.id) !=
              containingEndorsements3.end());
  EXPECT_EQ(endorsedBlockIndex->getEndorsedBy().size(), 1);

  // remove block
  AltBlock removeBlock = chain[20];
  alttree.removeSubtree(removeBlock.getHash());

  containingBlockIndex3 = alttree.getBlockIndex(endorsement3.containingHash);
  auto& containingEndorsements4 =
      containingBlockIndex3->getContainingEndorsements();
  EXPECT_TRUE(containingEndorsements4.find(endorsement3.id) !=
              containingEndorsements4.end());

  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  EXPECT_EQ(endorsedBlockIndex->getEndorsedBy().size(), 1);

  EXPECT_TRUE(alttree.setState(forkchain2.back().getHash(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(),
            popminer->vbk().getBestChain().tip()->getHash());
}

TEST_F(AltTreeFixture, compareTrees) {
  AltBlockTree alttree2 =
      AltBlockTree(altparam, vbkparam, btcparam, payloadsProvider);
  EXPECT_TRUE(alttree2.bootstrap(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), state));
  EXPECT_TRUE(alttree2.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), state));
  EXPECT_TRUE(cmp(alttree, alttree2));
  EXPECT_TRUE(cmp(alttree.vbk(), alttree2.vbk()));
  EXPECT_TRUE(cmp(alttree.vbk().btc(), alttree2.vbk().btc()));

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 1 blocks
  mineAltBlocks(1, chain);
  EXPECT_FALSE(cmp(alttree, alttree2, true));
  EXPECT_TRUE(cmp(alttree.vbk(), alttree2.vbk()));
  EXPECT_TRUE(cmp(alttree.vbk().btc(), alttree2.vbk().btc()));
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
  auto containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);
  PopData payloads1 =
      generateAltPayloads({tx1}, GetRegTestVbkBlock().getHash(), 0);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(validatePayloads(containingBlock.getHash(), payloads1));
  EXPECT_TRUE(state.IsValid());

  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  EXPECT_TRUE(containingIndex->isValid());
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(),
            popminer->vbk().getBestChain().tip()->getHash());

  ASSERT_DEATH(validatePayloads(containingBlock.getHash(), payloads1),

               "already contains payloads");
  containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  EXPECT_TRUE(containingIndex->isValid());
}

TEST_F(AltTreeFixture, invalidBlockIndex_test) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  // mine 10 blocks
  mineAltBlocks(10, chain);

  auto* index = alttree.getBlockIndex(chain.back().getHash());

  EXPECT_EQ(index->getHash(), alttree.getBestChain().tip()->getHash());

  index->setFlag(BLOCK_FAILED_POP);
  EXPECT_FALSE(index->isValid());

  EXPECT_FALSE(alttree.acceptBlockHeader(index->getHeader(), state));
}

TEST_F(AltTreeFixture, assertBlockSanity_test) {
  auto block = generateNextBlock(altparam.getBootstrapBlock());

  block.hash = block.previousBlock;

  EXPECT_EQ(block.getHash(), block.getPreviousBlock());

  ASSERT_DEATH(
      {
        bool ok = alttree.acceptBlockHeader(block, state);
        (void)ok;
      },
      "Previous block hash should NOT be equal to the current block hash");
}

TEST_F(AltTreeFixture, max_VbkBlocks_in_PopData_test) {
  //   std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  //   // mine VBK blocks
  //   popminer->mineVbkBlocks(MAX_POPDATA_VBK * 1.5);

  //   auto popData = generateAltPayloads({}, GetRegTestVbkBlock().getHash());

  //   ASSERT_GT(popData.context.size(), MAX_POPDATA_VBK);
}