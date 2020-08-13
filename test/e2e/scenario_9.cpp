// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * This scenario test the vbk duplicates validation in the same altblock chain
 * This test case is a similar test case as scenario_2
 *
 * endorse 2 valid altchain blocks
 * generate two popdata with the same vbkblocks
 */

struct Scenario9 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario9, scenario_9) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  generatePopTx(endorsedVbkBlock1->getHeader());
  auto* btcBlockTip1 = popminer->btc().getBestChain().tip();
  popminer->mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1);

  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  auto E1 = VbkEndorsement::fromContainer(vtbs[0]);
  auto E2 = VbkEndorsement::fromContainer(vtbs[1]);
  ASSERT_NE(E1.id, E2.id);

  // store vtbs in different altPayloads
  altPayloads1.vtbs = {vtbs[0]};
  fillVbkContext(altPayloads1.context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs[0].containingBlock.getHash(),
                 popminer->vbk());

  // Step 1
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(
      alttree.addPayloads(containingBlock.getHash(), altPayloads1, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  verifyEndorsementAdded(alttree.vbk(), E1);
  validateAlttreeIndexState(alttree, containingBlock, altPayloads1, true);

  auto* containinVbkBlock = alttree.vbk().getBlockIndex(vbkTip->getHash());

  EXPECT_TRUE(containinVbkBlock->getContainingEndorsements().count(
      VbkEndorsement::getId(vtbs[0])));
  EXPECT_FALSE(containinVbkBlock->getContainingEndorsements().count(
      VbkEndorsement::getId(vtbs[1])));

  // check btc tree state
  EXPECT_EQ(alttree.vbk().btc().getBestChain().tip()->getHash(),
            btcBlockTip1->getHash());

  mineAltBlocks(10, chain);
  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  PopData altPayloads2 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  altPayloads2.vtbs = {vtbs[1]};

  // Step 2
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_FALSE(
      alttree.addPayloads(containingBlock.getHash(), altPayloads2, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  validateAlttreeIndexState(
      alttree, containingBlock, altPayloads2, true, false);
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(), "VBK-duplicate");
  verifyEndorsementAdded(alttree.vbk(), E1);
}
