// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

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
 * - addPayloads for the alt block with invalid VTB which is contained in the
 * bootstrap vbk block
 */

struct Scenario6 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario6, AddPayloadsToGenesisBlock) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  AltTree test_alttree(altparam, vbkparam, btcparam, storagePayloads);

  // do not bootstrap VBK
  ASSERT_TRUE(test_alttree.bootstrap(state));
  ASSERT_TRUE(test_alttree.btc().bootstrapWithGenesis(state));

  // Step1
  // mine 10 Vbk blocks in the pop miner
  auto* vbkTip = popminer->mineVbkBlocks(10);
  ASSERT_EQ(*popminer->vbk().getBestChain().tip(), *vbkTip);
  auto* btcTip = popminer->mineBtcBlocks(40);
  ASSERT_EQ(*popminer->btc().getBestChain().tip(), *btcTip);

  // endorsed vbk block
  auto* endorsedBlock = vbkTip->getAncestor(vbkTip->height - 5);
  generatePopTx(*endorsedBlock->header);
  vbkTip = popminer->mineVbkBlocks(1);
  auto it = popminer->vbkPayloads.find(vbkTip->getHash());
  ASSERT_TRUE(it != popminer->vbkPayloads.end());
  ASSERT_EQ(it->second.size(), 1);

  auto vtb = it->second[0];

  // corrupt vtb
  std::vector<uint8_t> new_hash = {1, 2, 3, 9, 8, 2};
  vtb.transaction.blockOfProof.previousBlock = uint256(new_hash);

  // Step 2
  // bootsrap with the non genesis block
  EXPECT_TRUE(test_alttree.vbk().bootstrapWithChain(
      vbkTip->height, {*vbkTip->header}, state));

  VbkTx tx =
      popminer->createVbkTxEndorsingAltBlock(generatePublicationData(chain[0]));
  AltBlock containingAltBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingAltBlock);
  PopData altPayloads =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // put corrupted VTB
  altPayloads.vtbs.push_back(vtb);

  EXPECT_TRUE(test_alttree.acceptBlock(containingAltBlock, state));
  ASSERT_TRUE(test_alttree.addPayloads(
      containingAltBlock.getHash(), altPayloads, state));
  EXPECT_FALSE(test_alttree.setState(containingAltBlock.getHash(), state));

  EXPECT_EQ(*test_alttree.vbk().getBestChain().tip(), *vbkTip);
  EXPECT_EQ(test_alttree.vbk().getBestChain().blocksCount(), 1);
  EXPECT_EQ(test_alttree.vbk().getBestChain().tip()->payloadIds.size(), 0);
}
