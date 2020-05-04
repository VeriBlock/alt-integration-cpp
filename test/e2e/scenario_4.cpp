#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct Scenario4 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario4, scenario_4) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto vbkTip = *popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip.getAncestor(vbkTip.height - 10);
  auto* endorsedVbkBlock2 = vbkTip.getAncestor(vbkTip.height - 11);
  generatePopTx(*endorsedVbkBlock1->header);
  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  vbkTip = *popminer.mineVbkBlocks(50);
  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer.mineVbkBlocks(1);
  vbkTip = *containingVbkBlock2;

  auto vtbs1 = popminer.vbkPayloads[containingVbkBlock1->getHash()];
  auto vtbs2 = popminer.vbkPayloads[containingVbkBlock2->getHash()];

  ASSERT_EQ(vtbs1.size(), 1);
  ASSERT_EQ(vtbs2.size(), 1);
  ASSERT_NE(BtcEndorsement::fromContainer(vtbs1[0]).id,
            BtcEndorsement::fromContainer(vtbs2[0]).id);
  fillVTBContext(
      vtbs1[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(
      vtbs2[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer.vbk().getBestChain().tip();

  altPayloads1.popData.vtbs = {vtbs1[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());

  mineAltBlocks(10, chain);

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer.vbk().getBestChain().tip();

  altPayloads2.popData.vtbs = {vtbs2[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, containingVbkBlock1->getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer.vbk().getBestChain().tip();

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());
}
