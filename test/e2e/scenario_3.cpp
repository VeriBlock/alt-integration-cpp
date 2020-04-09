#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;
// clang-format off
/**
 * BTC:
 * o-o-o-o-o-o-o-o-o-o-o-o-B102 - 102 blocks (tip, longest)
 *
 *
 * VBK:
 *
 * o-o-o-o-o-o-o-o-o-o-o-o-[Height 67: 2 VTBs: Btc1 and Btc102] (tip)
 *
 *
 * ALT:
 * o-o-o-o-o-o-o-[Height 11: Payloads1 with VTB Btc1]-o-o-o-o-o-o-o-o-o-[Height 22: Payloads2 with VTB Btc102] - [Height 23: Payloads4 without VTBs]
 *                                                        \-[Height 18: Payloads3 without VTB]
 *
 * alt tree has VBK+BTC bootstrapped at the genesis blocks
 *
 * Step 1
 * proccess Alt block height 11 with one VTB with the Btc block 1
 * expect that Alt block btc tree state on the Btc block 1
 * expect that Vbk block 67 has 1 endorsement
 * Step 2
 * proccess Alt block height 22 with one VTB with the Btc block 102
 * expect that Alt block btc tree state on the Btc block 102
 * expect that Vbk block 67 has 2 endorsement
 * Step 3
 * proccess Alt block height 17 without VTBs
 * expect that Alt block btc tree state on the Btc block 1
 * expect that Vbk block 67 has 1 endorsement
 * Step 4
 * proccess Alt block height 23 without VTBs
 * expect that Alt block btc tree state on the Btc block 102
 * expect that Vbk block 67 has 2 endorsement
 */
// clang-format on

struct Scenario3 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario3, scenario_3) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip1 = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip1->getAncestor(vbkTip1->height - 10);
  auto* vbkForkPoint = vbkTip1->getAncestor(vbkTip1->height - 30);
  auto endorsedVbkBlock2 = popminer.mineVbkBlocks(*vbkForkPoint, 23);

  ASSERT_TRUE(popminer.vbk().getBestChain().contains(endorsedVbkBlock1));
  ASSERT_FALSE(popminer.vbk().getBestChain().contains(endorsedVbkBlock2));

  auto popTx1 = generatePopTx(endorsedVbkBlock1->header);
  popminer.mineBtcBlocks(10);
  auto popTx2 = generatePopTx(endorsedVbkBlock2->header);
  popminer.mineBtcBlocks(100);
  auto popTx3 = generatePopTx(endorsedVbkBlock1->header);
  popminer.vbkmempool.clear();

  auto* vbkTip2 = popminer.mineVbkBlocks(*endorsedVbkBlock2, {popTx2});

  // vbkTip1 heigher than vbkTip2
  ASSERT_GT(vbkTip1->height, vbkTip2->height);
  // but active chain on the vbkTip2 because this chain has endorsements
  ASSERT_EQ(vbkTip2->getHash(), popminer.vbk().getBestChain().tip()->getHash());

  vbkTip1 = popminer.mineVbkBlocks(*vbkTip1, {popTx1, popTx3});

  // now we switch active chain to the better endorsements
  ASSERT_GT(vbkTip1->height, vbkTip2->height);
  ASSERT_EQ(vbkTip1->getHash(), popminer.vbk().getBestChain().tip()->getHash());

  auto vtbs1 = popminer.vbkPayloads[vbkTip1->getHash()];
  auto vtbs2 = popminer.vbkPayloads[vbkTip2->getHash()];

  ASSERT_EQ(vtbs1.size(), 2);
  ASSERT_EQ(vtbs2.size(), 1);

  fillVTBContext(
      vtbs1[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(
      vtbs1[1], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(
      vtbs2[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  auto* btcContaininBlock1 =
      popminer.btc().getBlockIndex(vtbs1[0].transaction.blockOfProof.getHash());
  auto* btcContaininBlock2 =
      popminer.btc().getBlockIndex(vtbs1[1].transaction.blockOfProof.getHash());

  // check vtbs1[0] is better for scorring than vtbs1[1]
  ASSERT_LT(btcContaininBlock1->height, btcContaininBlock2->height);

  // mine 10 Alt blocks
  mineAltBlocks(10, chain);
  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip1->getHash());
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads1.vtbs = {vtbs1[1]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip1->getHash());

  auto* containingVbkBlock =
      alttree.vbk().getBlockIndex(vtbs1[1].containingBlock.getHash());

  // check endorsements
  EXPECT_TRUE(containingVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs1[0]).id) ==
              containingVbkBlock->containingEndorsements.end());

  EXPECT_TRUE(containingVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs1[1]).id) !=
              containingVbkBlock->containingEndorsements.end());

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip1->getHash());
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads2.vtbs = {vtbs2[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip2->getHash());

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip1->getHash());
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads3.vtbs = {vtbs1[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  EXPECT_TRUE(containingVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs1[0]).id) !=
              containingVbkBlock->containingEndorsements.end());

  EXPECT_TRUE(containingVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs1[1]).id) !=
              containingVbkBlock->containingEndorsements.end());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip1->getHash());
}
