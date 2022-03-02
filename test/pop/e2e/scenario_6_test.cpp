// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

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
  AltBlockTree test_alttree(
      altparam, vbkparam, btcparam, payloadsProvider, blockProvider);

  // do not bootstrap VBK
  EXPECT_NO_FATAL_FAILURE(test_alttree.bootstrap());
  EXPECT_NO_FATAL_FAILURE(
      test_alttree.btc().bootstrapWithGenesis(GetRegTestBtcBlock()));

  // Step1
  // mine 10 Vbk blocks in the pop miner
  auto* vbkTip = popminer.mineVbkBlocks(10);

  ASSERT_TRUE(cmp(*popminer.vbk().getBestChain().tip(), *vbkTip));
  auto* btcTip = popminer.mineBtcBlocks(40);

  ASSERT_TRUE(cmp(*popminer.btc().getBestChain().tip(), *btcTip));

  // endorsed vbk block
  auto* endorsedBlock = vbkTip->getAncestor(vbkTip->getHeight() - 5);
  auto vbkPopTx = generatePopTx(endorsedBlock->getHeader());
  vbkTip = popminer.mineVbkBlocks(1, {vbkPopTx});
  auto vtb = popminer.createVTB(vbkTip->getHeader(), vbkPopTx);

  // corrupt vtb
  std::vector<uint8_t> new_hash = {1, 2, 3, 9, 8, 2};
  vtb.transaction.blockOfProof.setPreviousBlock(new_hash);

  // Step 2
  // bootsrap with the non genesis block
  EXPECT_NO_FATAL_FAILURE(test_alttree.vbk().bootstrapWithChain(
      vbkTip->getHeight(), {vbkTip->getHeader()}));

  VbkTx tx =
      popminer.createVbkTxEndorsingAltBlock(generatePublicationData(chain[0]));
  AltBlock containingAltBlock = generateNextBlock(chain.back());
  chain.push_back(containingAltBlock);
  PopData altPayloads =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  // put corrupted VTB
  altPayloads.vtbs.push_back(vtb);

  EXPECT_TRUE(test_alttree.acceptBlockHeader(containingAltBlock, state));
  test_alttree.acceptBlock(containingAltBlock.getHash(), altPayloads);
  EXPECT_FALSE(test_alttree.setState(containingAltBlock.getHash(), state));

  // all payloads are marked valid as there's no correctly implemented
  // invalidation
  validateAlttreeIndexState(test_alttree,
                            containingAltBlock,
                            altPayloads,
                            /*payloads_validation =*/true);

  EXPECT_EQ(test_alttree.vbk().getBestChain().tip()->getHash(),
            vbkTip->getHash());
  EXPECT_EQ(test_alttree.vbk().getBestChain().blocksCount(), 1);
  auto& vtbids = test_alttree.vbk().getBestChain().tip()->getPayloadIds<VTB>();
  EXPECT_EQ(vtbids.size(), 0);
}
