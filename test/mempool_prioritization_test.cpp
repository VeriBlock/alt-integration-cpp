// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/mempool_block_tree.hpp"

using namespace altintegration;

struct MemPoolPrioritizationFixture : public ::testing::Test,
                                      public PopTestFixture {
  MemPoolPrioritizationFixture() : mempool_tree(alttree) {}
  MemPoolBlockTree mempool_tree;

  BlockIndex<BtcBlock>* mineBtcBlocks(const BlockIndex<BtcBlock>& tip,
                                      size_t amount) {
    EXPECT_NE(amount, 0);
    BlockIndex<BtcBlock>* blk = popminer->mineBtcBlocks(tip, 1);
    EXPECT_TRUE(alttree.btc().acceptBlock(blk->getHeader(), state));
    for (size_t i = 1; i < amount; ++i) {
      blk = popminer->mineBtcBlocks(*blk, 1);
      EXPECT_TRUE(alttree.btc().acceptBlock(blk->getHeader(), state));
    }
    return blk;
  }

  BlockIndex<BtcBlock>* mineBtcBlocks(size_t amount) {
    auto* tip = alttree.btc().getBestChain().tip();
    assert(tip);
    return mineBtcBlocks(*tip, amount);
  }

  BlockIndex<VbkBlock>* mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                      size_t amount) {
    EXPECT_NE(amount, 0);
    BlockIndex<VbkBlock>* blk = popminer->mineVbkBlocks(tip, 1);
    EXPECT_TRUE(alttree.vbk().acceptBlock(blk->getHeader(), state));
    EXPECT_GE(alttree.vbk().getBestChain().tip()->getHeight(),
              alttree.vbk().getParams().getGenesisBlock().height);
    for (size_t i = 1; i < amount; ++i) {
      blk = popminer->mineVbkBlocks(*blk, 1);
      EXPECT_TRUE(alttree.vbk().acceptBlock(blk->getHeader(), state));
      EXPECT_GE(alttree.vbk().getBestChain().tip()->getHeight(),
                alttree.vbk().getParams().getGenesisBlock().height);
    }
    return blk;
  }

  BlockIndex<VbkBlock>* mineVbkBlocks(size_t amount) {
    auto* tip = alttree.vbk().getBestChain().tip();
    assert(tip);
    return mineVbkBlocks(*tip, amount);
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto tx = PopTestFixture::generatePopTx(endorsedBlock);
    for (const auto& blk : tx.blockOfProofContext) {
      EXPECT_TRUE(alttree.btc().acceptBlock(blk, state));
    }
    EXPECT_TRUE(alttree.btc().acceptBlock(tx.blockOfProof, state));
    return tx;
  }
};

// Compare the same vtbs
TEST_F(MemPoolPrioritizationFixture, vtb_areStronglyEquivalent_scenario1_test) {
  // mine 65 VBK blocks
  auto* vbkTip = mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  EXPECT_TRUE(mempool_tree.areStronglyEquivalent(vtbs[0], vtbs[0]));
  EXPECT_EQ(mempool_tree.weaklyCompare(vtbs[0], vtbs[0]), 0);
}

// Compare vtbs that are contains in the same chain with the same vbkPop
// transactions
TEST_F(MemPoolPrioritizationFixture, vtb_areStronglyEquivalent_scenario2_test) {
  // mine 65 VBK blocks
  auto* vbkTip = mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb1 = vtbs[0];

  ASSERT_TRUE(popminer->vbkmempool.empty());
  popminer->vbkmempool.push_back(vbkPopTx);
  ASSERT_FALSE(popminer->vbkmempool.empty());
  vbkTip = mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_TRUE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));
  EXPECT_EQ(mempool_tree.weaklyCompare(vtb1, vtb2), 0);
}

// Compare vtbs that are contains in the different chains with the same vbkPop
// transactions
TEST_F(MemPoolPrioritizationFixture, vtb_areStronglyEquivalent_scenario3_test) {
  auto* vbkFork = mineVbkBlocks(50);
  auto* vbkTip = mineVbkBlocks(2);

  // endorse VBK blockss
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb1 = vtbs[0];

  ASSERT_TRUE(popminer->vbkmempool.empty());
  popminer->vbkmempool.push_back(vbkPopTx);
  ASSERT_FALSE(popminer->vbkmempool.empty());
  auto* vbkForkTip = mineVbkBlocks(*vbkFork, 1);
  ASSERT_EQ(vbkForkTip->getHeight(), vbkFork->getHeight() + 1);
  ASSERT_NE(vbkForkTip->getHash(),
            popminer->vbk().getBestChain().tip()->getHash());
  vtbs = popminer->vbkPayloads[vbkForkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_TRUE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));
  EXPECT_EQ(mempool_tree.weaklyCompare(vtb1, vtb2), 0);
}

// Compare vtbs that are not equal
TEST_F(MemPoolPrioritizationFixture, vtb_areStronglyEquivalent_scenario4_test) {
  // mine 65 vbk blocks
  auto* vbkTip = mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->getHeight() - 11);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());
  vbkTip = mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));

  EXPECT_TRUE(mempool_tree.areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(mempool_tree.areWeaklyEquivalent(vtb2, vtb1));

  EXPECT_GT(mempool_tree.weaklyCompare(vtb1, vtb2), 0);
  EXPECT_LT(mempool_tree.weaklyCompare(vtb2, vtb1), 0);
}

TEST_F(MemPoolPrioritizationFixture, vtb_areWeaklyEquivalent_scenario1_test) {
  // mine 65 VBK blocks
  auto* vbkTip = mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  mineBtcBlocks(1);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));

  EXPECT_TRUE(mempool_tree.areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_TRUE(mempool_tree.areWeaklyEquivalent(vtb2, vtb1));

  EXPECT_GT(mempool_tree.weaklyCompare(vtb1, vtb2), 0);
  EXPECT_LT(mempool_tree.weaklyCompare(vtb2, vtb1), 0);
}

TEST_F(MemPoolPrioritizationFixture, vtb_areWeaklyEquivalent_scenario2_test) {
  // mine 65 VBK blocks
  auto* vbkTip = mineVbkBlocks(65);

  // mine 10 Btc blocks
  auto* btcFork = mineBtcBlocks(10);

  // endorse VBK blocks
  const auto* endorsedVbkBlock = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  // mine btc fork
  mineBtcBlocks(*btcFork, 10);
  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock->getHeader());
  vbkTip = mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));

  EXPECT_FALSE(mempool_tree.areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areWeaklyEquivalent(vtb2, vtb1));

  ASSERT_DEATH(mempool_tree.weaklyCompare(vtb1, vtb2),
               "vtbs should be weakly equivalent");
  ASSERT_DEATH(mempool_tree.weaklyCompare(vtb2, vtb1),
               "vtbs should be weakly equivalent");
}

TEST_F(MemPoolPrioritizationFixture, vtb_areWeaklyEquivalent_scenario3_test) {
  // mine 65 vbk blocks
  auto* vbkTip =
      mineVbkBlocks(popminer->vbk().getParams().getKeystoneInterval() * 3);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(
      vbkTip->getHeight() - popminer->vbk().getParams().getKeystoneInterval() +
      1);
  const auto* endorsedVbkBlock2 =
      vbkTip->getAncestor(popminer->vbk().getParams().getKeystoneInterval());

  auto vbkPopTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  vbkTip = mineVbkBlocks(1);
  auto& vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx1.getHash());

  auto vtb1 = vtbs[0];

  auto vbkPopTx2 = generatePopTx(endorsedVbkBlock2->getHeader());
  vbkTip = mineVbkBlocks(1);
  vtbs = popminer->vbkPayloads[vbkTip->getHash()];
  ASSERT_EQ(vtbs.size(), 1);
  ASSERT_EQ(vtbs[0].transaction.getHash(), vbkPopTx2.getHash());

  auto vtb2 = vtbs[0];

  ASSERT_NE(vtb1.getId(), vtb2.getId());

  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areStronglyEquivalent(vtb2, vtb1));

  EXPECT_FALSE(mempool_tree.areWeaklyEquivalent(vtb1, vtb2));
  EXPECT_FALSE(mempool_tree.areWeaklyEquivalent(vtb2, vtb1));

  ASSERT_DEATH(mempool_tree.weaklyCompare(vtb1, vtb2),
               "vtbs should be weakly equivalent");
  ASSERT_DEATH(mempool_tree.weaklyCompare(vtb2, vtb1),
               "vtbs should be weakly equivalent");
}

TEST_F(MemPoolPrioritizationFixture, atv_areStronglyEquivalent_scenario1_test) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  ASSERT_EQ(altPayloads1.atvs.size(), 1);
  ASSERT_TRUE(altPayloads1.vtbs.empty());

  auto atv = altPayloads1.atvs[0];

  ASSERT_EQ(atv.transaction.getHash(), tx.getHash());

  ASSERT_TRUE(mempool_tree.areStronglyEquivalent(atv, atv));
}

TEST_F(MemPoolPrioritizationFixture, atv_areStronglyEquivalent_scenario2_test) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];
  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));

  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());
  PopData altPayloads2 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  ASSERT_EQ(altPayloads1.atvs.size(), 1);
  ASSERT_TRUE(altPayloads1.vtbs.empty());
  ASSERT_EQ(altPayloads2.atvs.size(), 1);
  ASSERT_TRUE(altPayloads2.vtbs.empty());

  auto atv1 = altPayloads1.atvs[0];
  auto atv2 = altPayloads2.atvs[0];

  ASSERT_EQ(atv1.transaction.getHash(), tx.getHash());
  ASSERT_EQ(atv2.transaction.getHash(), tx.getHash());

  ASSERT_FALSE(mempool_tree.areStronglyEquivalent(atv1, atv2));
  ASSERT_FALSE(mempool_tree.areStronglyEquivalent(atv2, atv1));
}