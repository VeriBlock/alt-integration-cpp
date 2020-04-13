#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>

using namespace altintegration;

/**
 * BTC:     /-o-o-A53-o-o - 55
 * o-o-o-o-o-o-o-B53-o-o-o-o - 57 (tip, longest)
 *         50
 * A = contains endorsement of vAe, in block 53 of chain A
 * B = contains endorsement of vAe, in block 53 of chain B
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
 * expect that ALTVBK tree has all blocks from VBK chain A, until vAc71, including
 * expect that ALTVBK tip is vAc71
 * Step 2
 * send VTB_vBc71 (only VTB) in ALT block 102 (in chain A of ALT)
 * expect that ALTBTC tree knows all blocks from both chains (A+B) until
 * containing blocks,
 * expect that ALTVBK tree knows all blocks from both chains (A+B) until
 * containing blocks.
 * expect that ALTBTC tip is B53
 * expect that ALTVBK tip is vBc71
 * expect that ALT tip is 102
 * Step 3
 * remove ALT block 102
 * expect that VTB_vBc71 is removed
 * expect that ALTBTC tree has all blocks from BTC chain A, until A53, including
 * expect that ALTBTC tip is A53
 * expect that ALTVBK tree has all blocks from VBK chain A, until vAc71, including
 * expect that ALTVBK tip is vAc71
 * Step 4
 * remove ALT block 101
 * expect that VTB_vAc71 is removed
 * expect that ALTBTC is at bootstrap
 * expect that ALTVBK is at bootstrap
 * expect that ALT is at 100
 */
struct Scenario1 : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btcAtip;
  BlockIndex<BtcBlock>* btcBtip;
  BlockIndex<VbkBlock>* vbkAtip;
  BlockIndex<VbkBlock>* vbkBtip;
  BlockIndex<AltBlock>* alttip;

  std::vector<AltBlock> altchain;

  Scenario1() {
    auto* btcFork = popminer.mineBtcBlocks(50);
    btcAtip = popminer.mineBtcBlocks(*btcFork, 2);
    btcBtip = popminer.mineBtcBlocks(*btcFork, 2);

    auto* vbkFork = popminer.mineVbkBlocks(50);
    // build up more blocks since POP fork resolution only works after
    // keystone interval has been passed
    auto* vbkAendorsed = popminer.mineVbkBlocks(*vbkFork, 20);
    auto* vbkBendorsed = popminer.mineVbkBlocks(*vbkFork, 20);

    auto btctxA = popminer.createBtcTxEndorsingVbkBlock(vbkAendorsed->header);
    auto* btcAContaining = popminer.mineBtcBlocks(*btcAtip, 1);
    btcAtip = popminer.mineBtcBlocks(*btcAContaining, 2);

    auto btctxB = popminer.createBtcTxEndorsingVbkBlock(vbkBendorsed->header);
    auto* btcBContaining = popminer.mineBtcBlocks(*btcBtip, 1);
    btcBtip = popminer.mineBtcBlocks(*btcBContaining, 4);

    EXPECT_EQ(btcBtip->getHash(),
              popminer.btc().getBestChain().tip()->getHash());

    auto vbktxA = popminer.createVbkPopTxEndorsingVbkBlock(
        btcAContaining->header,
        btctxA,
        vbkAendorsed->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    vbkAtip = popminer.mineVbkBlocks(*vbkAendorsed, 5);

    auto vbktxB = popminer.createVbkPopTxEndorsingVbkBlock(
        btcBContaining->header,
        btctxB,
        vbkBendorsed->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    vbkBtip = popminer.mineVbkBlocks(*vbkBendorsed, 5);

    altchain = {altparam.getBootstrapBlock()};
    mineAltBlocks(100, altchain);
  }
};

AltPayloads generateAltPayloadsEmpty(const AltBlock& containing,
                                     const AltBlock& endorsed) {
  AltPayloads alt;
  alt.hasAtv = false;
  alt.containingBlock = containing;
  alt.endorsed = endorsed;
  return alt;
}

TEST_F(Scenario1, scenario_1) {
  // Step 1
  ASSERT_EQ(vbkAtip->height, vbkBtip->height);
  EXPECT_EQ(vbkBtip->getHash(), popminer.vbk().getBestChain().tip()->getHash());

  AltBlock endorsedBlock = altchain[90];
  AltBlock containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  AltPayloads altPayloadsVBA71 =
      generateAltPayloadsEmpty(containingBlock, endorsedBlock);

  auto vtbsVBA71 = popminer.vbkPayloads[vbkAtip->getAncestor(71)->getHash()];
  fillVTBContext(
      vtbsVBA71[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  altPayloadsVBA71.vtbs = {vtbsVBA71[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloadsVBA71}, state));
  EXPECT_TRUE(state.IsValid());

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  auto altbtcMap = alttree.vbk().btc();
  auto* block = btcAtip->getAncestor(53);
  size_t blockCount = 0;
  while (true) {
    if (block == nullptr) break;
    auto *index = altbtcMap.getBlockIndex(block->getHash());
    EXPECT_NE(index, nullptr);
    block = block->pprev;
    blockCount++;
  }

  // expect that ALTBTC tip is A53
  EXPECT_EQ(blockCount, 54);
  EXPECT_EQ(btcAtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  auto altvbkMap = alttree.vbk();
  auto* blockVbk = vbkAtip->getAncestor(71);
  blockCount = 0;
  while (true) {
    if (blockVbk == nullptr) break;
    auto* index = altvbkMap.getBlockIndex(blockVbk->getHash());
    EXPECT_NE(index, nullptr);
    blockVbk = blockVbk->pprev;
    blockCount++;
  }

  // expect that ALTVBK tip is vAc71
  EXPECT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());

  // Step 2
  endorsedBlock = altchain[90];
  containingBlock = generateNextBlock(*altchain.rbegin());
  altchain.push_back(containingBlock);

  AltPayloads altPayloadsVBB71 =
      generateAltPayloadsEmpty(containingBlock, endorsedBlock);

  // send VTB_vBc71 (only VTB) in ALT block 102 (in chain A of ALT)
  auto vtbsVBB71 = popminer.vbkPayloads[vbkBtip->getAncestor(71)->getHash()];
  fillVTBContext(
      vtbsVBB71[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  altPayloadsVBB71.vtbs = {vtbsVBB71[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloadsVBB71}, state));
  EXPECT_TRUE(state.IsValid());

  // expect that ALTBTC tree knows all blocks from both chains (A+B) until
  // containing blocks
  altbtcMap = alttree.vbk().btc();
  auto* blockA = btcAtip->getAncestor(53);
  auto* blockB = btcBtip->getAncestor(53);
  blockCount = 0;
  ///TODO: this expectation fails for BTCA 53, 52, 51
  while (true) {
    if (blockA == nullptr) break;
    auto* index = altbtcMap.getBlockIndex(blockA->getHash());
    EXPECT_NE(index, nullptr);
    blockA = blockA->pprev;
    blockCount++;
  }
  while (true) {
    if (blockB == nullptr) break;
    auto* index = altbtcMap.getBlockIndex(blockB->getHash());
    EXPECT_NE(index, nullptr);
    blockB = blockB->pprev;
    blockCount++;
  }
  EXPECT_EQ(blockCount, 54 * 2);

  // expect that ALTVBK tree knows all blocks from both chains(A + B)
  // until containing blocks.
  altvbkMap = alttree.vbk();
  blockVbk = vbkAtip->getAncestor(71);
  blockCount = 0;
  while (true) {
    if (blockVbk == nullptr) break;
    auto* index = altvbkMap.getBlockIndex(blockVbk->getHash());
    EXPECT_NE(index, nullptr);
    blockVbk = blockVbk->pprev;
    blockCount++;
  }

  blockVbk = vbkBtip->getAncestor(71);
  while (true) {
    if (blockVbk == nullptr) break;
    auto* index = altvbkMap.getBlockIndex(blockVbk->getHash());
    EXPECT_NE(index, nullptr);
    blockVbk = blockVbk->pprev;
    blockCount++;
  }
  EXPECT_EQ(blockCount, 72 * 2);

  // expect that ALTBTC tip is B53
  EXPECT_EQ(btcBtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());
  // expect that ALTVBK tip is vBc71
  ///TODO: this expectation fails
  EXPECT_EQ(vbkBtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());
  // expect that ALT tip is 102
  EXPECT_EQ(altchain.size(), 103);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 102);

  // Step 3
  // remove ALT block 102
  auto lastBlock = *altchain.rbegin();
  auto altContext =
      alttree.getBlockIndex(lastBlock.getHash())->containingContext;
  EXPECT_TRUE(altContext.size() > 0);
  bool vtbFound = false;
  for (auto c : altContext) {
    for (auto v : c.vtbs) {
      if (v == vtbsVBB71[0]) {
        vtbFound = true;
      }
    }
  }
  EXPECT_TRUE(vtbFound);

  alttree.removePayloads(lastBlock, {altPayloadsVBB71});
  altchain.pop_back();
  EXPECT_EQ(altchain.size(), 102);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 101);

  // expect that VTB_vBc71 is removed
  lastBlock = *altchain.rbegin();
  altContext = alttree.getBlockIndex(lastBlock.getHash())->containingContext;
  EXPECT_TRUE(altContext.size() > 0);
  vtbFound = false;
  for (auto c : altContext) {
    for (auto v : c.vtbs) {
      if (v == vtbsVBB71[0]) {
        vtbFound = true;
      }
    }
  }
  EXPECT_FALSE(vtbFound);

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53, including
  ///TODO: this expectation fails
  altbtcMap = alttree.vbk().btc();
  block = btcBtip->getAncestor(53);
  blockCount = 0;
  while (true) {
    if (block == nullptr) break;
    auto* index = altbtcMap.getBlockIndex(block->getHash());
    EXPECT_NE(index, nullptr);
    block = block->pprev;
    blockCount++;
  }

  // expect that ALTBTC tip is A53
  ///TODO: this expectation fails
  EXPECT_EQ(blockCount, 54);
  EXPECT_EQ(btcAtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  altvbkMap = alttree.vbk();
  blockVbk = vbkAtip->getAncestor(71);
  blockCount = 0;
  while (true) {
    if (blockVbk == nullptr) break;
    auto* index = altvbkMap.getBlockIndex(blockVbk->getHash());
    EXPECT_NE(index, nullptr);
    blockVbk = blockVbk->pprev;
    blockCount++;
  }

  // expect that ALTVBK tip is vAc71
  EXPECT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());

  // Step 4
  // remove ALT block 101
  lastBlock = *altchain.rbegin();
  altContext = alttree.getBlockIndex(lastBlock.getHash())->containingContext;
  EXPECT_TRUE(altContext.size() > 0);
  vtbFound = false;
  for (auto c : altContext) {
    for (auto v : c.vtbs) {
      if (v == vtbsVBA71[0]) {
        vtbFound = true;
      }
    }
  }
  EXPECT_TRUE(vtbFound);

  alttree.removePayloads(lastBlock, {altPayloadsVBA71});
  altchain.pop_back();
  EXPECT_EQ(altchain.size(), 101);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 100);

  // expect that VTB_vAc71 is removed
  lastBlock = *altchain.rbegin();
  altContext = alttree.getBlockIndex(lastBlock.getHash())->containingContext;
  EXPECT_TRUE(altContext.size() > 0);
  vtbFound = false;
  for (auto c : altContext) {
    for (auto v : c.vtbs) {
      if (v == vtbsVBA71[0]) {
        vtbFound = true;
      }
    }
  }
  EXPECT_FALSE(vtbFound);
}
