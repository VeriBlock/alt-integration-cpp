// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct Scenario10 : public ::testing::Test, public PopTestFixture {};

TEST_F(Scenario10, scenario_10) {
  std::vector<AltBlock> chainA = {altparam.getBootstrapBlock()};

  // mine 10 blocks to the chainA
  mineAltBlocks(10, chainA, /*connectBlocks=*/true, /*setState=*/false);

  // mine 65 VBK blocks
  auto* vbkTip = popminer->mineVbkBlocks(65);
  // mine 50 BTC blocks
  popminer->mineBtcBlocks(50);

  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  popminer->mineBtcBlocks(100);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  vbkTip = popminer->mineVbkBlocks(1, {vbkPopTx1, vbkPopTx2});

  auto vtb1 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx1);
  auto vtb2 = popminer->createVTB(vbkTip->getHeader(), vbkPopTx2);

  auto E1 = VbkEndorsement::fromContainer(vtb1);
  auto E2 = VbkEndorsement::fromContainer(vtb2);
  ASSERT_NE(E1.id, E2.id);

  PopData payloads1 =
      generateAltPayloads({}, alttree.vbk().getBestChain().tip()->getHash(), 0);

  ASSERT_EQ(payloads1.context.back().getHash(), vbkTip->getHash());

  AltBlock containingBlock1 = generateNextBlock(chainA.back());
  chainA.push_back(containingBlock1);

  VBK_LOG_DEBUG("Step 1");
  ASSERT_TRUE(alttree.acceptBlockHeader(containingBlock1, state));
  ASSERT_TRUE(AddPayloads(containingBlock1.getHash(), payloads1));
  ASSERT_TRUE(alttree.setState(containingBlock1.getHash(), state));
  ASSERT_TRUE(state.IsValid());
  ASSERT_EQ(alttree.vbk()
                .getBlockIndex(vbkTip->getHash())
                ->getPayloadIds<VTB>()
                .size(),
            0);

  PopData payloads2 =
      generateAltPayloads({}, alttree.vbk().getBestChain().tip()->getHash(), 0);
  payloads2.vtbs = {vtb1, vtb2};

  AltBlock containingBlock2 = generateNextBlock(chainA.back());
  chainA.push_back(containingBlock2);

  VBK_LOG_DEBUG("Step 2");
  ASSERT_TRUE(alttree.acceptBlockHeader(containingBlock2, state));
  ASSERT_TRUE(AddPayloads(containingBlock2.getHash(), payloads2));
  ASSERT_TRUE(alttree.setState(containingBlock2.getHash(), state));
  ASSERT_TRUE(state.IsValid());
  ASSERT_EQ(alttree.vbk()
                .getBlockIndex(vbkTip->getHash())
                ->getPayloadIds<VTB>()
                .size(),
            2);

  VBK_LOG_DEBUG("Step 3");
  auto batch = storage.generateWriteBatch();
  auto writer = adaptors::BlockBatchImpl(*batch);
  saveTrees(alttree, writer);
  batch->writeBatch();

  VBK_LOG_DEBUG("Step 4");
  AltBlockTree reloadedAltTree{this->altparam,
                               this->vbkparam,
                               this->btcparam,
                               payloadsProvider,
                               blockProvider};

  ASSERT_TRUE(reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock(),
                                                         this->state));
  ASSERT_TRUE(reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(),
                                                         this->state));
  ASSERT_TRUE(reloadedAltTree.bootstrap(this->state));

  ASSERT_TRUE(loadTrees(reloadedAltTree, blockProvider, state));

  ASSERT_TRUE(reloadedAltTree.getBlockIndex(chainA.back().getHash()) !=
              nullptr);

  ASSERT_EQ(reloadedAltTree.vbk()
                .getBlockIndex(vbkTip->getHash())
                ->getPayloadIds<VTB>()
                .size(),
            2);

  ASSERT_TRUE(
      reloadedAltTree.setState(chainA[chainA.size() - 5].getHash(), state));
  ASSERT_TRUE(state.IsValid());
}