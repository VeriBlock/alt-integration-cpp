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
 * o-o-o-o-o-o-vBe52-vBc53-o-o-o-o - 55  (tip, better pop score, B is on main chain)
 *         50
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
 */
struct Scenario1 : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btcAtip;
  BlockIndex<BtcBlock>* btcBtip;
  BlockIndex<VbkBlock>* vbkAtip;
  BlockIndex<VbkBlock>* vbkBtip;
  BlockIndex<AltBlock>* alttip;

  Scenario1() {
    auto *btcFork = popminer.mineBtcBlocks(50);
    btcAtip = popminer.mineBtcBlocks(*btcFork, 2);

    auto vbkFork = popminer.mineVbkBlocks(50);
    vbkAtip = popminer.mineVbkBlocks(*vbkFork, 2);

    auto btctxA = popminer.createBtcTxEndorsingVbkBlock(vbkAtip->header);
    auto btcA = popminer.mineBtcBlocks(1);
    btcAtip = popminer.mineBtcBlocks(2);

    auto vbktxA = popminer.endorseVbkBlock(vbkAtip->header, getLastKnownBlocks(alttree.btc(), 1)[0], state);
  }


};

// TEST_F(Scenario1, )