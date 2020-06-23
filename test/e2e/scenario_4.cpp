// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct Scenario4 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario4, scenario_4) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto vbkTip = *popminer->mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip.getAncestor(vbkTip.height - 10);
  auto* endorsedVbkBlock2 = vbkTip.getAncestor(vbkTip.height - 11);
  generatePopTx(*endorsedVbkBlock1->header);
  auto* containingVbkBlock1 = popminer->mineVbkBlocks(1);
  vbkTip = *popminer->mineVbkBlocks(50);
  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer->mineVbkBlocks(1);
  vbkTip = *containingVbkBlock2;

  auto vtbs1 = popminer->vbkPayloads[containingVbkBlock1->getHash()];
  auto vtbs2 = popminer->vbkPayloads[containingVbkBlock2->getHash()];

  ASSERT_EQ(vtbs1.size(), 1);
  ASSERT_EQ(vtbs2.size(), 1);
  ASSERT_NE(VbkEndorsement::fromContainer(vtbs1[0]).id,
            VbkEndorsement::fromContainer(vtbs2[0]).id);
  fillVbkContext(vtbs1[0].context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs1[0].containingBlock.getHash(),
                 popminer->vbk());
  fillVbkContext(vtbs2[0].context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbs2[0].containingBlock.getHash(),
                 popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer->vbk().getBestChain().tip();

  altPayloads1.vtbs = {vtbs1[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock.hash, altPayloads1, state));
  EXPECT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());

  mineAltBlocks(10, chain);

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  PopData altPayloads2 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer->vbk().getBestChain().tip();

  altPayloads2.vtbs = {vtbs2[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock.hash, altPayloads2, state));
  EXPECT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);
  PopData altPayloads3 =
      generateAltPayloads({tx}, containingVbkBlock1->getHash());

  // new tip is the next block after vbkTip
  ASSERT_EQ(popminer->vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip.getHash());
  vbkTip = *popminer->vbk().getBestChain().tip();

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock.hash, altPayloads3, state));
  EXPECT_TRUE(alttree.setState(containingBlock.hash, state));
  EXPECT_TRUE(state.IsValid());

  // check vbk tree state
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip.getHash());
}
