#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct Scenario4 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario4, scenario_3) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(endorsedVbkBlock1->header);
  popminer.mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->header);

  vbkTip = popminer.mineVbkBlocks(1);

  auto vtbs = popminer.vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(BtcEndorsement::fromContainer(vtbs[0]).id,
            BtcEndorsement::fromContainer(vtbs[1]).id);
  fillVTBContext(vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip->getHash());
  vbkTip = popminer.vbk().getBestChain().tip();

  altPayloads1.vtbs = {vtbs[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip->getHash());
}
