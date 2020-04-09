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
 *          /-o-vAe52-vAc53-o-o-o-o - 57
 * o-o-o-o-o-o-vBe52-vBc53-o-o-o-o - 55  (tip, better pop score, B is on main
 * chain) 50
 *
 * vAe - endorsed block in chain A (block 52)
 * vAc - block that contains endorsement of block vAe (block 53)
 * vBe - endorsed block in chain B (block 52)
 * vBc - block that contains endorsement of block vBe (block 53)
 *
 *
 * ALT: (BTC block tree)
 * o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o 100 blocks, no ATVs or VTBs
 * alt tree has VBK+BTC bootstrapped at the genesis blocks
 *
 * Step 1
 * send VTB_vAc53 (only VTB, set flag hasAtv=false in alt payloads) in alt block 101 (chain A of ALT)
 * expect that ALTBTC tree has all blocks from BTC chain A, until A53, including
 * expect that ALTBTC tip is A53
 * expect that ALTVBK tree has all blocks from VBK chain A, until vAc53, including
 * expect that ALTVBK tip is vAc53
 */
struct Scenario1 : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btcAtip;
  BlockIndex<BtcBlock>* btcBtip;
  BlockIndex<VbkBlock>* vbkAtip;
  BlockIndex<VbkBlock>* vbkBtip;
  BlockIndex<AltBlock>* alttip;

  Scenario1() {
    auto* btcFork = popminer.mineBtcBlocks(50);
    btcAtip = popminer.mineBtcBlocks(*btcFork, 2);
    btcBtip = popminer.mineBtcBlocks(*btcFork, 2);

    auto* vbkFork = popminer.mineVbkBlocks(50);
    vbkAtip = popminer.mineVbkBlocks(*vbkFork, 2);
    vbkBtip = popminer.mineVbkBlocks(*vbkFork, 2);

    auto btctxA = popminer.createBtcTxEndorsingVbkBlock(vbkAtip->header);
    auto* btcAContaining = popminer.mineBtcBlocks(*btcAtip, 1);
    btcAtip = popminer.mineBtcBlocks(*btcAContaining, 2);

    auto btctxB = popminer.createBtcTxEndorsingVbkBlock(vbkBtip->header);
    auto* btcBContaining = popminer.mineBtcBlocks(*btcBtip, 1);
    btcBtip = popminer.mineBtcBlocks(*btcBContaining, 4);

    EXPECT_EQ(btcBtip->getHash(),
              popminer.btc().getBestChain().tip()->getHash());

    auto vbktxA = popminer.createVbkPopTxEndorsingVbkBlock(
        btcAContaining->header,
        btctxA,
        vbkAtip->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    //vbkAtip = popminer.mineVbkBlocks(*vbkAtip, {vbktxA});
    vbkAtip = popminer.mineVbkBlocks(*vbkAtip, 5);

    auto vbktxB = popminer.createVbkPopTxEndorsingVbkBlock(
        btcBContaining->header,
        btctxB,
        vbkBtip->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    //vbkBtip = popminer.mineVbkBlocks(*vbkBtip, {vbktxB});
    vbkBtip = popminer.mineVbkBlocks(*vbkBtip, 5);

    std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
    mineAltBlocks(100, chain);
  }
};

TEST_F(Scenario1, block101Vtb) {
  ASSERT_EQ(vbkAtip->height, vbkBtip->height);
  ASSERT_EQ(vbkAtip->getHash(), popminer.vbk().getBestChain().tip()->getHash());
}