#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;
// clang-format off
/**
 * VBK:
 *
 * o-o-o-o-o-o-o-o-o-o-o-o--o-o-[Height 66, popTx1 endorsement in the Btc block 1, popTx2 endorsement in the Btc block 103]-o-o-o (tip, 69 blocks)
 *           \[Height 59, popTx3 endorsement in the Btc block 12] (59 blocks, fork)   
 *
 * ALT:
 * o-o-o-o-o-o-o-[Height 11: Payloads1 with VTB popTx2]-[Height 12: Payloads2 with VTB popTx3]-[Height 12: Payloads3 with VTB popTx1]
 *
 * alt tree has VBK+BTC bootstrapped at the genesis blocks
 *
 * Step 1
 * proccess all Vbk endorsement/payloads, so as a result we have one fork with one endorsement,
 * and active chain with two endorsements/payloads
 * Step 2
 * proccess Alt block height 11 with one VTB with the popTx2
 * expect that Vbk tree state on the Vbk 67 block
 * expect that Vbk block 66 has 1 endorsement
 * Step 3
 * proccess Alt block height 12 with one VTB with the popTx3 (also have been updated the tip of the highest chain with one block, because we have created one ATV)
 * expect that Vbk tree state on the Vbk 59 block (fork chain)
 * expect that Vbk block 66 has 1 endorsement
 * expect that Vbk block 59 has 1 endorsement
 * Step 4
 * proccess Alt block height 13 with one VTB with the popTx1
 * expect that Vbk tree state on the Vbk 69 block (acive chain)
 * expect that Vbk block 66 has 2 endorsement
 * expect that Vbk block 59 has 1 endorsement
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
  // Step 1
  auto popTx1 = generatePopTx(*endorsedVbkBlock1->header);
  popminer.mineBtcBlocks(10);
  auto popTx2 = generatePopTx(*endorsedVbkBlock2->header);
  popminer.mineBtcBlocks(100);
  auto popTx3 = generatePopTx(*endorsedVbkBlock1->header);
  popminer.vbkmempool.clear();

  auto* vbkTip2 = popminer.mineVbkBlocks(*endorsedVbkBlock2, {popTx2});

  // vbkTip1 heigher than vbkTip2
  ASSERT_GT(vbkTip1->height, vbkTip2->height);
  // but active chain on the vbkTip2 because this chain has endorsements
  ASSERT_EQ(*vbkTip2, *popminer.vbk().getBestChain().tip());

  vbkTip1 = popminer.mineVbkBlocks(*vbkTip1, {popTx1, popTx3});

  // now we switch active chain to the better endorsements
  ASSERT_GT(vbkTip1->height, vbkTip2->height);
  ASSERT_EQ(*vbkTip1, *popminer.vbk().getBestChain().tip());

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

  // Step 2
  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(*popminer.vbk().getBestChain().tip()->pprev, *vbkTip1);
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads1.popData.vtbs = {vtbs1[1]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(*alttree.vbk().getBestChain().tip(), *vbkTip1);

  auto* containingVbkBlock =
      alttree.vbk().getBlockIndex(vtbs1[1].containingBlock.getHash());

  // check endorsements
  EXPECT_FALSE(containingVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs1[0])));
  EXPECT_TRUE(containingVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs1[1])));

  // Step 3
  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(popminer.vbk().getBestChain().tip()->pprev->getHash(),
            vbkTip1->getHash());
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads2.popData.vtbs = {vtbs2[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(), vbkTip2->getHash());

  // Step 4
  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // new tip is the next block after vbkTip1
  ASSERT_EQ(*popminer.vbk().getBestChain().tip()->pprev, *vbkTip1);
  vbkTip1 = popminer.vbk().getBestChain().tip();

  // store vtbs in different altPayloads
  altPayloads3.popData.vtbs = {vtbs1[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  EXPECT_TRUE(state.IsValid());

  // check endorsements
  EXPECT_TRUE(containingVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs1[0])));
  EXPECT_TRUE(containingVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs1[1])));

  EXPECT_EQ(*alttree.vbk().getBestChain().tip(), *vbkTip1);
}
