#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;
/**
 *
 * VBK:
 * try to bootstrap block and then addPayloads with the corrupted VTB for this
 * bootstrap block
 *
 * Step 1:
 * - mine 10 vbk blocks in the popminer
 * - mine 40 btc blocks in the popminer
 * - endorse vbk block
 * - corrupt endorsement
 * Step 2:
 * - boostrap test_alttree with the vbk block which contains endorsement
 * - endorse an alt block
 * - put corrupted VTB to the altPayloads
 * - addPayloads for the alt block with invalid VTB which contains in the
 * bootstrap vbk block
 */

struct Scenario6 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario6, scenario_6) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  AltTree test_alttree(altparam, vbkparam, btcparam);

  // do not
  ASSERT_TRUE(test_alttree.bootstrap(state));
  ASSERT_TRUE(test_alttree.btc().bootstrapWithGenesis(state));

  // Step1
  // mine 10 Vbk blocks in the pop miner
  auto* vbkTip = popminer.mineVbkBlocks(10);
  ASSERT_EQ(*popminer.vbk().getBestChain().tip(), *vbkTip);
  auto* btcTip = popminer.mineBtcBlocks(40);
  ASSERT_EQ(*popminer.btc().getBestChain().tip(), *btcTip);

  // endorsed vbk block
  auto* endorsedBlock = vbkTip->getAncestor(vbkTip->height - 5);
  generatePopTx(*endorsedBlock->header);
  vbkTip = popminer.mineVbkBlocks(1);
  auto it = popminer.vbkPayloads.find(vbkTip->getHash());
  ASSERT_TRUE(it != popminer.vbkPayloads.end());
  ASSERT_EQ(it->second.size(), 1);

  auto vtb = it->second[0];
  // corrupt vtb
  std::vector<uint8_t> new_hash = {1, 2, 3};
  vtb.transaction.blockOfProof.previousBlock = uint256(new_hash);

  // Step 2
  // bootsrap with the non genesis block
  EXPECT_TRUE(test_alttree.vbk().bootstrapWithChain(
      vbkTip->height, {*vbkTip->header}, state));

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(chain[0]));
  AltBlock containingAltBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingAltBlock);
  AltPayloads altPayloads = generateAltPayloads(
      tx, containingAltBlock, chain[0], vbkparam.getGenesisBlock().getHash());

  // put corrupted VTB
  altPayloads.altPopTx.vtbs.push_back(vtb);

  EXPECT_TRUE(test_alttree.acceptBlock(containingAltBlock, state));
  EXPECT_FALSE(
      test_alttree.addPayloads(containingAltBlock, {altPayloads}, state));

  EXPECT_EQ(*test_alttree.vbk().getBestChain().tip(), *vbkTip);
  EXPECT_EQ(test_alttree.vbk().getBestChain().blocksCount(), 1);
  EXPECT_EQ(test_alttree.vbk()
                .getBestChain()
                .tip()
                ->containingContext.btc_context.size(),
            0);
}
