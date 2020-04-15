#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct AltTreeFixture : public ::testing::Test, public PopTestFixture {};

TEST_F(AltTreeFixture, invalidate_block_test1) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 50 blocks
  mineAltBlocks(50, chain);

  // make fork
  auto forkchain = chain;
  forkchain.resize(30);
  mineAltBlocks(15, forkchain);

  AltBlock endorsedBlock = chain[5];

  // generate endorsements
  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
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
  tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  containingBlock = generateNextBlock(*forkchain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  VbkEndorsement endorsement2 = VbkEndorsement::fromContainer(altPayloads2);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  auto* containingBlockIndex2 =
      alttree.getBlockIndex(endorsement2.containingHash);
  EXPECT_TRUE(
      containingBlockIndex2->containingEndorsements.find(endorsement2.id) !=
      containingBlockIndex2->containingEndorsements.end());
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 2);

  // remove block
  AltBlock removeBlock = chain[10];
  alttree.invalidateBlockByHash(removeBlock.getHash());

  endorsedBlockIndex = alttree.getBlockIndex(endorsement2.endorsedHash);
  EXPECT_EQ(endorsedBlockIndex->endorsedBy.size(), 0);
}
