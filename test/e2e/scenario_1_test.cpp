// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>

using namespace altintegration;

// clang-format off
/**
 * BTC:     /-o-o-A53-o-o - 55
 * o-o-o-o-o-o-o-o-o-B55-o-o - 57 (tip, longest)
 *         50
 * A = contains endorsement of vAe, in block 53 of chain A
 * B = contains endorsement of vAe, in block 55 of chain B
 *
 * VBK:
 *          /-o-vAe70-vAc71-o-o-o-o - 75
 * o-o-o-o-o-o-vBe70-vBc71-o-o-o-o - 75  (tip, better pop score, B is on main
 * chain) 50
 *
 * vAe - endorsed block in chain A (block 70)
 * vAc - block that contains endorsement of block vAe (block 71)
 * vBe - endorsed block in chain B (block 70)
 * vBc - block that contains endorsement of block vBe (block 71)
 *
 *
 * ALT: (BTC block tree)
 * o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o 100 blocks, no ATVs or VTBs
 * alt tree has VBK+BTC bootstrapped at the genesis blocks
 *
 * Step 1
 * send VTB_vAc71 (only VTB, set flag hasAtv=false in alt payloads) in alt block
 * 101 (chain A of ALT)
 * expect that ALTBTC tree has all blocks from BTC chain A, until A53, including
 * expect that ALTBTC tip is A53
 * expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
 * including
 * expect that ALTVBK tip is vAc71
 * Step 2
 * send VTB_vBc71 (only VTB) in ALT block 102 (in chain A of ALT)
 * expect that ALTBTC tree knows all blocks from chain B until block B55
 * expect that ALTVBK tree knows all blocks from both chains (A+B) until
 * containing blocks.
 * expect that ALTBTC tip is B55
 * expect that ALTVBK tip is vBc71
 * expect that ALT tip is 102
 * Step 3
 * remove ALT block 102
 * expect that VTB_vBc71 is removed
 * expect that ALTBTC tree has all blocks from BTC chain A, until A53, including
 * expect that ALTBTC tip is A53
 * expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
 * including
 * expect that ALTVBK tip is vAc71
 * Step 4
 * remove ALT block 101
 * expect that VTB_vAc71 is removed
 * expect that ALTBTC is at bootstrap
 * expect that ALTVBK is at bootstrap
 * expect that ALT is at 100
 */
// clang-format on

struct Scenario1 : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btcAtip;
  BlockIndex<BtcBlock>* btcBtip;
  BlockIndex<VbkBlock>* vbkAtip;
  BlockIndex<VbkBlock>* vbkBtip;
  BlockIndex<VbkBlock>* vbkFork;

  std::vector<AltBlock> altchain;

  Scenario1() {
    auto* btcFork = popminer->mineBtcBlocks(50);
    btcAtip = popminer->mineBtcBlocks(*btcFork, 2);
    btcBtip = popminer->mineBtcBlocks(*btcFork, 4);

    vbkFork = popminer->mineVbkBlocks(50);
    // build up more blocks since POP fork resolution only works after
    // keystone interval has been passed
    auto* vbkAendorsed = popminer->mineVbkBlocks(*vbkFork, 20);
    auto* vbkBendorsed = popminer->mineVbkBlocks(*vbkFork, 20);

    auto btctxA =
        popminer->createBtcTxEndorsingVbkBlock(vbkAendorsed->getHeader());
    auto* btcAContaining = popminer->mineBtcBlocks(*btcAtip, 1);
    btcAtip = popminer->mineBtcBlocks(*btcAContaining, 2);

    auto btctxB =
        popminer->createBtcTxEndorsingVbkBlock(vbkBendorsed->getHeader());
    auto* btcBContaining = popminer->mineBtcBlocks(*btcBtip, 1);
    btcBtip = popminer->mineBtcBlocks(*btcBContaining, 2);

    EXPECT_EQ(btcBtip->getHash(),
              popminer->btc().getBestChain().tip()->getHash());
    EXPECT_EQ(btcAtip->getHeight(), 55);
    EXPECT_EQ(btcBtip->getHeight(), 57);

    auto vbktxA = popminer->createVbkPopTxEndorsingVbkBlock(
        btcAContaining->getHeader(),
        btctxA,
        vbkAendorsed->getHeader(),
        popminer->getBtcParams().getGenesisBlock().getHash());
    vbkAtip = popminer->mineVbkBlocks(*vbkAendorsed, 5);
    EXPECT_EQ(btcAtip->getHeight(), 55);
    EXPECT_EQ(btcBtip->getHeight(), 57);

    auto vbktxB = popminer->createVbkPopTxEndorsingVbkBlock(
        btcBContaining->getHeader(),
        btctxB,
        vbkBendorsed->getHeader(),
        popminer->getBtcParams().getGenesisBlock().getHash());
    EXPECT_EQ(btcAtip->getHeight(), 55);
    EXPECT_EQ(btcBtip->getHeight(), 57);
    vbkBtip = popminer->mineVbkBlocks(*vbkBendorsed, 5);
    EXPECT_EQ(btcAtip->getHeight(), 55);
    EXPECT_EQ(btcBtip->getHeight(), 57);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(100, altchain);
  }

  template <typename Block, typename ChainParams>
  size_t checkBlocksExisting(const BlockTree<Block, ChainParams>& tree,
                             const BlockIndex<Block>* tip) {
    size_t blockCount = 0;
    while (tip) {
      auto* index = tree.getBlockIndex(tip->getHash());
      EXPECT_NE(index, nullptr);
      tip = tip->pprev;
      blockCount++;
    }
    return blockCount;
  }
};

TEST_F(Scenario1, scenario_1) {
  // Step 1
  ASSERT_EQ(vbkAtip->getHeight(), vbkBtip->getHeight());
  ASSERT_TRUE(cmp(*vbkBtip, *popminer->vbk().getBestChain().tip()));

  AltBlock endorsedBlock = altchain[90];
  AltBlock containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  PopData altPayloadsVBA71;

  auto vtbsVBA71 = popminer->vbkPayloads[vbkAtip->getAncestor(71)->getHash()];
  fillVbkContext(altPayloadsVBA71.context,
                 vbkparam.getGenesisBlock().getHash(),
                 vtbsVBA71[0].containingBlock.getHash(),
                 popminer->vbk());
  altPayloadsVBA71.vtbs = {vtbsVBA71[0]};
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  ASSERT_TRUE(AddPayloads(containingBlock.getHash(), altPayloadsVBA71))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  EXPECT_TRUE(state.IsValid());
  ASSERT_NE(btcAtip, nullptr);
  ASSERT_GE(btcAtip->getHeight(), 53);

  validateAlttreeIndexState(alttree, containingBlock, altPayloadsVBA71);

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  auto* btcA53 = btcAtip->getAncestor(53);
  ASSERT_TRUE(btcA53);
  size_t blockCount = checkBlocksExisting(alttree.btc(), btcA53);

  // expect that ALTBTC tip is A53
  ASSERT_EQ(blockCount, 54);
  EXPECT_TRUE(cmp(*btcA53, *alttree.vbk().btc().getBestChain().tip()));

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  blockCount = checkBlocksExisting(alttree.vbk(), vbkAtip->getAncestor(71));

  // expect that ALTVBK tip is vAc71
  ASSERT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(),
            altchain.rbegin()->getHash());

  // Step 2
  endorsedBlock = altchain[90];
  containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  PopData altPayloadsVBB71;

  // send VTB_vBc71 (only VTB) in ALT block 102 (in chain A of ALT)
  auto vtbsVBB71 = popminer->vbkPayloads[vbkBtip->getAncestor(71)->getHash()];
  fillVbkContext(altPayloadsVBB71.context,
                 vbkFork->getHash(),
                 vtbsVBB71[0].containingBlock.getHash(),
                 popminer->vbk());
  altPayloadsVBB71.vtbs = {vtbsVBB71[0]};
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  ASSERT_TRUE(AddPayloads(containingBlock.getHash(), altPayloadsVBB71))
      << state.toString();
  ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state))
      << state.toString();
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(),
            altchain.rbegin()->getHash());
  EXPECT_TRUE(state.IsValid());

  validateAlttreeIndexState(alttree, containingBlock, altPayloadsVBB71);

  // expect that ALTBTC tree knows all blocks from chain B until block B55
  blockCount =
      checkBlocksExisting(alttree.vbk().btc(), btcBtip->getAncestor(55));
  EXPECT_EQ(blockCount, 56);

  // expect that ALTVBK tree knows all blocks from both chains(A + B)
  // until containing blocks.
  blockCount = checkBlocksExisting(alttree.vbk(), vbkAtip->getAncestor(71));
  blockCount += checkBlocksExisting(alttree.vbk(), vbkBtip->getAncestor(71));
  EXPECT_EQ(blockCount, 72 * 2);

  // expect that ALTBTC tip is B55
  EXPECT_EQ(btcBtip->getAncestor(55)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());
  // expect that ALTVBK tip is vBc71
  EXPECT_TRUE(
      cmp(*vbkBtip->getAncestor(71), *alttree.vbk().getBestChain().tip()));
  // expect that ALT tip is 102
  EXPECT_EQ(altchain.size(), 103);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 102);

  // Step 3
  // remove ALT block 102
  auto lastBlock = *altchain.rbegin();
  alttree.removeSubtree(lastBlock.getHash());
  altchain.pop_back();
  EXPECT_EQ(altchain.size(), 102);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 101);

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  blockCount = checkBlocksExisting(alttree.vbk().btc(), btcA53);
  // expect that ALTBTC tip is A53
  EXPECT_EQ(blockCount, 54);
  EXPECT_TRUE(cmp(*btcAtip->getAncestor(53),
                  *alttree.vbk().btc().getBestChain().tip()));

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  blockCount = checkBlocksExisting(alttree.vbk(), vbkAtip->getAncestor(71));

  // expect that ALTVBK tip is vAc71
  EXPECT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());

  // Step 4
  // remove ALT block 101
  lastBlock = *altchain.rbegin();
  alttree.removeSubtree(lastBlock.getHash());
  altchain.pop_back();

  // expect that ALT is at 100
  EXPECT_EQ(altchain.size(), 101);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 100);

  // expect that ALTBTC is at bootstrap
  EXPECT_EQ(btcparam.getGenesisBlock().getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());
  // expect that ALTVBK is at bootstrap
  EXPECT_EQ(vbkparam.getGenesisBlock().getHash(),
            alttree.vbk().getBestChain().tip()->getHash());
}
