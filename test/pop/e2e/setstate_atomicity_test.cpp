// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * Mine chain A: endorse 2 valid altchain blocks
 * Mine chain B: endorse 1 block, add an invalid VTB to this block
 * Activate chain A
 * Attempt to activate chain B
 * Check that the chain B activation attempt failed and left the state unchanged
 */

struct SetStateAtomicity : public ::testing::Test, public PopTestFixture {};

TEST_F(SetStateAtomicity, setStateAtomicity) {
  AltBlock chainA = alttree.getBestChain().tip()->getHeader();
  // mine and activate the primary valid chain
  auto altForkPoint = chainA;

  chainA = generateNextBlock(chainA);
  auto payloads = endorseAltBlock({altForkPoint}, 1);
  ASSERT_TRUE(alttree.acceptBlockHeader(chainA, state)) << state.toString();
  ASSERT_TRUE(AddPayloads(chainA.getHash(), {payloads})) << state.toString();

  // make a copy that we will use later to create corrupted payloads
  VTB corruptedVtb = payloads.vtbs.at(0);

  chainA = generateNextBlock(chainA);
  payloads = endorseAltBlock({altForkPoint}, 1);
  payloads.context.erase(
      payloads.context.begin(),
      payloads.context.begin() + (payloads.context.size() - 2));
  ASSERT_TRUE(alttree.acceptBlockHeader(chainA, state)) << state.toString();
  ASSERT_TRUE(AddPayloads(chainA.getHash(), payloads)) << state.toString();

  // corrupted payloads
  std::vector<uint8_t> invalid_hash = {1, 2, 3, 9, 8, 2};
  corruptedVtb.transaction.blockOfProof.setPreviousBlock(invalid_hash);

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(altForkPoint));
  AltBlock corruptedAltBlock = generateNextBlock(altForkPoint);
  PopData corruptedPayloads =
      generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  corruptedPayloads.vtbs.push_back(corruptedVtb);

  ASSERT_TRUE(alttree.acceptBlockHeader(corruptedAltBlock, state))
      << state.toString();
  ASSERT_TRUE(AddPayloads(corruptedAltBlock.getHash(), corruptedPayloads))
      << state.toString();

  auto chainB = corruptedAltBlock;

  EXPECT_TRUE(alttree.setState(chainA.getHash(), state)) << state.toString();

  validateAlttreeIndexState(alttree, chainA, payloads);

  // attempting to set the state to a corrupted block should fail and leave the
  // tree unaltered
  auto* originalBtcTip = alttree.btc().getBestChain().tip();
  auto* originalVbkTip = alttree.vbk().getBestChain().tip();
  auto* originalAltTip = alttree.getBestChain().tip();
  EXPECT_EQ(originalAltTip->getHeader(), chainA);

  EXPECT_FALSE(alttree.setState(chainB.getHash(), state)) << state.toString();

  // all payloads are marked valid as there's no correctly implemented
  // invalidation
  validateAlttreeIndexState(
      alttree, chainB, corruptedPayloads, /*payloads_validation =*/true);

  EXPECT_EQ(originalBtcTip, alttree.btc().getBestChain().tip());
  EXPECT_EQ(originalVbkTip, alttree.vbk().getBestChain().tip());
  EXPECT_EQ(originalAltTip, alttree.getBestChain().tip());
}
