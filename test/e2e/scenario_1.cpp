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
 * 101 (chain A of ALT) expect that ALTBTC tree has all blocks from BTC chain A,
 * until A53, including expect that ALTBTC tip is A53 expect that ALTVBK tree
 * has all blocks from VBK chain A, until vAc71, including expect that ALTVBK
 * tip is vAc71 Step 2 send VTB_vBc53 (only VTB) in ALT block 102 (in chain A of
 * ALT) expect that ALTBTC tree knows all blocks from both chains (A+B) until
 * containing blocks, expect that ALTVBK tree knows all blocks from both chains
 * (A+B) until containing blocks. expect that ALTBTC tip is B53 expect that
 * ALTVBK tip is vBc53 expect that ALT tip is 102
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

  AltPayloads altPayloads1 =
      generateAltPayloadsEmpty(containingBlock, endorsedBlock);

  auto vtbs1 = popminer.vbkPayloads[vbkAtip->getAncestor(71)->getHash()];
  fillVTBContext(
      vtbs1[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  altPayloads1.vtbs = {vtbs1[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloads1}, state));
  EXPECT_TRUE(state.IsValid());

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  auto altbtcMap = alttree.vbk().btc().getAllBlocks();
  auto* block = btcAtip->getAncestor(53);
  size_t blockCount = 0;
  while (true) {
    if (block == nullptr) break;
    auto it = altbtcMap.find(block->getHash());
    EXPECT_NE(it, altbtcMap.end());
    block = block->pprev;
    blockCount++;
  }

  // expect that ALTBTC tip is A53
  EXPECT_EQ(blockCount, 54);
  EXPECT_EQ(btcAtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  auto altvbkMap = alttree.vbk().getAllBlocks();
  auto* blockVbk = vbkAtip->getAncestor(71);
  blockCount = 0;
  while (true) {
    if (blockVbk == nullptr) break;
    auto it = altvbkMap.find(blockVbk->getHash().trimLE<uint96::size()>());
    EXPECT_NE(it, altvbkMap.end());
    blockVbk = blockVbk->pprev;
    blockCount++;
  }

  // expect that ALTVBK tip is vAc71
  EXPECT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());

  // Step 2
}
