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

struct Scenario2 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario2, scenario_2) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  AltPayloads altPayloads1 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(*endorsedVbkBlock1->header);
  auto* btcBlockTip1 = popminer.btc().getBestChain().tip();
  popminer.mineBtcBlocks(100);
  generatePopTx(*endorsedVbkBlock2->header);
  auto* btcBlockTip2 = popminer.btc().getBestChain().tip();

  vbkTip = popminer.mineVbkBlocks(1);

  auto& vtbs = popminer.vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(BtcEndorsement::fromContainer(vtbs[0]).id,
            BtcEndorsement::fromContainer(vtbs[1]).id);
  fillVTBContext(vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  // store vtbs in different altPayloads
  altPayloads1.popData.vtbs = {vtbs[0]};

  // Step 1
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());
  auto* containinVbkBlock = alttree.vbk().getBlockIndex(vbkTip->getHash());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[0])));
  EXPECT_FALSE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[1])));

  // check btc tree state
  EXPECT_EQ(*alttree.vbk().btc().getBestChain().tip(), *btcBlockTip1);

  mineAltBlocks(10, chain);
  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  altPayloads2.popData.vtbs = {vtbs[1]};
  // Step 2
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads2}, state));
  EXPECT_TRUE(state.IsValid());

  containinVbkBlock = alttree.vbk().getBlockIndex(vbkTip->getHash());
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[0])));
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[1])));

  // check btc tree state
  EXPECT_EQ(*alttree.vbk().btc().getBestChain().tip(), *btcBlockTip2);

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);
  AltPayloads altPayloads3 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  // Step 3
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads3}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[0])));
  EXPECT_FALSE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[1])));

  // check btc tree state
  EXPECT_EQ(*alttree.vbk().btc().getBestChain().tip(), *btcBlockTip1);

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads4 = generateAltPayloads(
      tx, containingBlock, endorsedBlock, vbkparam.getGenesisBlock().getHash());

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads4}, state));
  EXPECT_TRUE(state.IsValid());

  containinVbkBlock = alttree.vbk().getBlockIndex(vbkTip->getHash());
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[0])));
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.count(
      BtcEndorsement::getId(vtbs[1])));

  // check btc tree state
  EXPECT_EQ(*alttree.vbk().btc().getBestChain().tip(), *btcBlockTip2);
}
