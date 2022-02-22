// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

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
  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  auto* btcBlockTip1 = popminer.btc().getBestChain().tip();
  popminer.mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer.mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer.createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer.createVTB(vbkTip->getHeader(), vbkPopTx2);

  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  ASSERT_NE(E1.id, E2.id);

  // store vtbs in different altPayloads
  altPayloads1.vtbs = {vtb1};
  fillVbkContext(altPayloads1.context,
                 GetRegTestVbkBlock().getHash(),
                 vtb1.containingBlock.getHash(),
                 popminer.vbk());

  VBK_LOG_DEBUG("Step 1");
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads1));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  verifyEndorsementAdded(alttree.vbk(), E1);
  validateAlttreeIndexState(alttree, containingBlock, altPayloads1, true);

  auto* containinVbkBlock = alttree.vbk().getBlockIndex(vbkTip->getHash());

  EXPECT_TRUE(containinVbkBlock->getContainingEndorsements().count(
      VbkEndorsement::getId(vtb1)));
  EXPECT_FALSE(containinVbkBlock->getContainingEndorsements().count(
      VbkEndorsement::getId(vtb2)));

  // check btc tree state
  EXPECT_EQ(alttree.vbk().btc().getBestChain().tip()->getHash(),
            btcBlockTip1->getHash());

  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);
  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);
  PopData altPayloads2 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  altPayloads2.vtbs = {vtb2};

  VBK_LOG_DEBUG("Step 2");
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_FALSE(AddPayloads(containingBlock.getHash(), altPayloads2))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "VBK-duplicate");
  EXPECT_FALSE(state.IsValid());

  EXPECT_FALSE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  validateAlttreeIndexState(alttree, containingBlock, altPayloads2, true);

  verifyEndorsementAdded(alttree.vbk(), E1);
}
