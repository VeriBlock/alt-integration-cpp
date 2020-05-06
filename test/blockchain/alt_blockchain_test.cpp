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
  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  VbkEndorsement endorsement1 = VbkEndorsement::fromContainer(altPayloads1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  auto* endorsedBlockIndex = alttree.getBlockIndex(endorsement1.endorsedHash);
  auto* containingBlockIndex1 =
      alttree.getBlockIndex(endorsement1.containingHash);
  EXPECT_TRUE(
      containingBlockIndex1->containingEndorsements.find(endorsement1.id) !=
      containingBlockIndex1->containingEndorsements.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  // generate endorsements
  tx = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(*forkchain1.rbegin());
  forkchain1.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  VbkEndorsement endorsement2 = VbkEndorsement::fromContainer(altPayloads2);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state))
      << state.toString();
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  auto* containingBlockIndex2 =
      alttree.getBlockIndex(endorsement2.containingHash);
  EXPECT_TRUE(
      containingBlockIndex2->containingEndorsements.find(endorsement2.id) !=
      containingBlockIndex2->containingEndorsements.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 2);

  tx = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(*forkchain2.rbegin());
  forkchain2.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  VbkEndorsement endorsement3 = VbkEndorsement::fromContainer(altPayloads3);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement3.endorsedHash);
  auto* containingBlockIndex3 =
      alttree.getBlockIndex(endorsement3.containingHash);
  EXPECT_TRUE(
      containingBlockIndex3->containingEndorsements.find(endorsement3.id) !=
      containingBlockIndex3->containingEndorsements.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 3);

  // remove block
  AltBlock removeBlock = chain[20];
  alttree.removeSubtree(removeBlock.getHash());

  containingBlockIndex3 = alttree.getBlockIndex(endorsement3.containingHash);
  EXPECT_TRUE(
      containingBlockIndex3->containingEndorsements.find(endorsement3.id) !=
      containingBlockIndex3->containingEndorsements.end());

  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 1);

  EXPECT_TRUE(alttree.setState(forkchain2.rbegin()->getHash(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(),
            popminer.vbk().getBestChain().tip()->getHash());
}
