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
  BlockIndex<AltBlock>* alttip;

  std::vector<AltBlock> altchain;

  Scenario1() {
    auto* btcFork = popminer.mineBtcBlocks(50);
    btcAtip = popminer.mineBtcBlocks(*btcFork, 2);
    btcBtip = popminer.mineBtcBlocks(*btcFork, 4);

    auto* vbkFork = popminer.mineVbkBlocks(50);
    // build up more blocks since POP fork resolution only works after
    // keystone interval has been passed
    auto* vbkAendorsed = popminer.mineVbkBlocks(*vbkFork, 20);
    auto* vbkBendorsed = popminer.mineVbkBlocks(*vbkFork, 20);

    auto btctxA = popminer.createBtcTxEndorsingVbkBlock(*vbkAendorsed->header);
    auto* btcAContaining = popminer.mineBtcBlocks(*btcAtip, 1);
    btcAtip = popminer.mineBtcBlocks(*btcAContaining, 2);

    auto btctxB = popminer.createBtcTxEndorsingVbkBlock(*vbkBendorsed->header);
    auto* btcBContaining = popminer.mineBtcBlocks(*btcBtip, 1);
    btcBtip = popminer.mineBtcBlocks(*btcBContaining, 2);

    EXPECT_EQ(btcBtip->getHash(),
              popminer.btc().getBestChain().tip()->getHash());

    auto vbktxA = popminer.createVbkPopTxEndorsingVbkBlock(
        *btcAContaining->header,
        btctxA,
        *vbkAendorsed->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    vbkAtip = popminer.mineVbkBlocks(*vbkAendorsed, 5);

    auto vbktxB = popminer.createVbkPopTxEndorsingVbkBlock(
        *btcBContaining->header,
        btctxB,
        *vbkBendorsed->header,
        popminer.getBtcParams().getGenesisBlock().getHash());
    vbkBtip = popminer.mineVbkBlocks(*vbkBendorsed, 5);

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

  bool altTreeFindVtb(const VTB& vtb) {
    auto lastBlock = *altchain.rbegin();
    auto* index = alttree.getBlockIndex(lastBlock.getHash());
    EXPECT_NE(index, nullptr);
    auto altContext = index->containingContext;
    for (const auto& v : altContext.vtbs) {
      if (v == PartialVTB::fromVTB(vtb)) {
        return true;
      }
    }
    return false;
  }
};

AltPayloads generateAltPayloadsEmpty(const AltBlock& containing,
                                     const AltBlock& endorsed) {
  PopData popData;
  popData.hasAtv = false;

  AltPayloads alt;
  alt.popData = popData;
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
  altPayloadsVBA71.popData.vtbs = {vtbsVBA71[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloadsVBA71}, state));
  EXPECT_TRUE(state.IsValid());

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  size_t blockCount =
      checkBlocksExisting(alttree.vbk().btc(), btcAtip->getAncestor(53));

  // expect that ALTBTC tip is A53
  EXPECT_EQ(blockCount, 54);
  EXPECT_EQ(btcAtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  blockCount = checkBlocksExisting(alttree.vbk(), vbkAtip->getAncestor(71));

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
  altPayloadsVBB71.popData.vtbs = {vtbsVBB71[0]};
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {altPayloadsVBB71}, state));
  EXPECT_TRUE(state.IsValid());

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
  EXPECT_EQ(vbkBtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());
  // expect that ALT tip is 102
  EXPECT_EQ(altchain.size(), 103);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 102);

  // Step 3
  EXPECT_TRUE(altTreeFindVtb(vtbsVBB71[0]));

  // remove ALT block 102
  auto lastBlock = *altchain.rbegin();
  alttree.invalidateBlockByHash(lastBlock.getHash());
  altchain.pop_back();
  EXPECT_EQ(altchain.size(), 102);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 101);

  // expect that VTB_vBc71 is removed
  EXPECT_FALSE(altTreeFindVtb(vtbsVBB71[0]));

  // expect that ALTBTC tree has all blocks from BTC chain A, until A53,
  // including
  blockCount =
      checkBlocksExisting(alttree.vbk().btc(), btcAtip->getAncestor(53));
  // expect that ALTBTC tip is A53
  EXPECT_EQ(blockCount, 54);
  EXPECT_EQ(btcAtip->getAncestor(53)->getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());

  // expect that ALTVBK tree has all blocks from VBK chain A, until vAc71,
  // including
  blockCount = checkBlocksExisting(alttree.vbk(), vbkAtip->getAncestor(71));

  // expect that ALTVBK tip is vAc71
  EXPECT_EQ(blockCount, 72);
  EXPECT_EQ(vbkAtip->getAncestor(71)->getHash(),
            alttree.vbk().getBestChain().tip()->getHash());

  // Step 4
  // remove ALT block 101
  EXPECT_TRUE(altTreeFindVtb(vtbsVBA71[0]));
  lastBlock = *altchain.rbegin();
  alttree.invalidateBlockByHash(lastBlock.getHash());
  altchain.pop_back();

  // expect that ALT is at 100
  EXPECT_EQ(altchain.size(), 101);
  EXPECT_EQ(altchain.at(altchain.size() - 1).height, 100);

  // expect that VTB_vAc71 is removed
  EXPECT_FALSE(altTreeFindVtb(vtbsVBA71[0]));

  // expect that ALTBTC is at bootstrap
  EXPECT_EQ(btcparam.getGenesisBlock().getHash(),
            alttree.vbk().btc().getBestChain().tip()->getHash());
  // expect that ALTVBK is at bootstrap
  EXPECT_EQ(vbkparam.getGenesisBlock().getHash(),
            alttree.vbk().getBestChain().tip()->getHash());
}
