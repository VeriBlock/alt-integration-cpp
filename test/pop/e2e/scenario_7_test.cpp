// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "pop/util/pop_test_fixture.hpp"

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
  mineAltBlocks(10, chain, /*connectBlocks=*/true);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());

  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx1});
  auto vtb1 = popminer->createVTB(containingVbkBlock1->getHeader(), vbkPopTx1);

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock1));
  auto* block1 = popminer->mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer->createATV(block1->getHeader(), tx1);

  popminer->mineBtcBlocks(100);
  popminer->mineVbkBlocks(54);

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1, {vbkPopTx2});
  auto vtb2 = popminer->createVTB(containingVbkBlock2->getHeader(), vbkPopTx2);

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock2));
  auto* block2 = popminer->mineVbkBlocks(1, {tx2});
  ATV atv2 = popminer->createATV(block2->getHeader(), tx2);

  PopData popData;
  popData.atvs = {atv1, atv2};
  popData.vtbs = {vtb1, vtb2};

  fillVbkContext(
      popData.context, GetRegTestVbkBlock().getHash(), popminer->vbk());

  auto containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData);
}
