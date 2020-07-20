// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * Mine the common part of the chains, chain.back() is the fork point
 * Mine chains A and B of equal length, crossing a keystone boundary
 *
 * Mine the last block of chain B:
 *     endorse 1 block after the last keystone
 *     create multiple VTBs
 *     this should cause chain B to win fork resolution
 *
 * Split the generated payloads into parts that will go into chains A and B:
 *     chain A gets the VBK context and the first VTB
 *     chain B gets the ATV and remaining VTBs
 *     chain A ends up being valid
 *     chain B would appear valid only if applied in addition to chain A
 *
 * Mine and activate the last block of chain A adding its part of the payloads
 * Mine the last block of chain B adding its part of the payloads
 *
 * Check that fork resolution causes state corruption due to deciding that
 * chain B is better, and unapplying chain A that chain B depends on for context
 * to be valid
 */

struct StandaloneValidation : public ::testing::Test, public PopTestFixture {};

TEST_F(StandaloneValidation, standaloneValidationDoesNotHappen) {
  // mine chains up to the fork point
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  mineAltBlocks(altparam.getKeystoneInterval() / 2, chain);

  // mine both chains save for the last block
  auto chainA = chain;
  auto chainB = chain;

  // cross the keystone boundary
  mineAltBlocks(altparam.getKeystoneInterval(), chainB);
  mineAltBlocks(altparam.getKeystoneInterval(), chainA);

  // generate the last chainB block that endorses the previous one
  auto endorsedBlockB = chainB.back();
  chainB.push_back(generateNextBlock(chainB.back()));
  auto payloadsB = endorseAltBlock({endorsedBlockB}, 10);

  // split the payloads: chain B is left with the latest VTBs
  PopData payloadsA = payloadsB;

  payloadsA.atvs.clear();
  payloadsA.vtbs.erase(payloadsA.vtbs.begin() + 1, payloadsA.vtbs.end());

  payloadsB.context.clear();
  payloadsB.vtbs.erase(payloadsB.vtbs.begin());

  // mine the last block of chainA with payloads and activate it
  mineAltBlocks(1, chainA);
  EXPECT_TRUE(alttree.addPayloads(chainA.back(), {payloadsA}, state));
  EXPECT_TRUE(alttree.setState(chainA.back().getHash(), state));
  EXPECT_TRUE(state.IsValid());

  // add the last block of chainB with payloads
  EXPECT_TRUE(alttree.acceptBlock(chainB.back(), state));
  EXPECT_TRUE(alttree.addPayloads(chainB.back(), {payloadsB}, state));

  // fork resolution decides that chainB is better
  // but switching to chainB causes state corruption
  // "not a leaf block" while trying to unapply the first VTB
  ASSERT_DEATH(
      alttree.comparePopScore(chainA.back().getHash(), chainB.back().getHash()),
      "");
}
