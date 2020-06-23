// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;
/**
 * Describe test case:
 * We generate 2 vtbs and 2 atvs and put it in 2 separate AltPaylaods, so
 * altPayloads1 contains vtb1, atv1
 * altPayloads2 contains vtb2, atv2
 * and simple call of the addPayloads should process smoothly
 */

struct Scenario7 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario7, scenario_7) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);

  generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer->vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer->generateATV(tx1, containingVbkBlock1->getHash(), state);

  vbkTip = popminer->vbk().getBestChain().tip();

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  ASSERT_EQ(popminer->vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer->vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVbkContext(vtb2, vbkTip->getHash(), popminer->vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer->generateATV(tx2, containingVbkBlock2->getHash(), state);

  PopData popData = createPopData(0, {atv1, atv2}, {vtb1, vtb2});

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, popData, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
}
