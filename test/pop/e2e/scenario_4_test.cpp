// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

struct Scenario4 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario4, scenario_4) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1, {vbkPopTx1});
  vbkTip = popminer->mineVbkBlocks(50);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1, {vbkPopTx2});
  vbkTip = containingVbkBlock2;

  auto vtb1 = popminer->createVTB(containingVbkBlock1->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(containingVbkBlock2->getHeader(), vbkPopTx2);

  ASSERT_NE(VbkEndorsement::fromContainer(vtb1).id,
            VbkEndorsement::fromContainer(vtb2).id);

  // mine 10 blocks
  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);
  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);
  PopData altPayloads1 =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip->getHash());
  vbkTip = popminer->vbk().getBestChain().tip();

  altPayloads1.vtbs = {vtb1};
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads1));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, altPayloads1);

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip->getHash());

  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);
  PopData altPayloads2 =
      generateAltPayloads({tx}, alttree.vbk().getBestChain().tip()->getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip->getHash());
  vbkTip = popminer->vbk().getBestChain().tip();

  altPayloads2.vtbs = {vtb2};
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads2));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, altPayloads2);

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip->getHash());

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(chain2.back());
  chain2.push_back(containingBlock);
  PopData altPayloads3 = generateAltPayloads(
      {tx}, alttree.vbk().getBestChain().tip()->pprev->getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip->getHash());
  vbkTip = popminer->vbk().getBestChain().tip();

  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), altPayloads3));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, altPayloads3);

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip->getHash());
}
