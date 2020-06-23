// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct Scenario8 : public ::testing::Test, public PopTestFixture {};

void validityFlagCheck(const BlockIndex<VbkBlock>& blockIndex, bool expected) {
  EXPECT_EQ(blockIndex.isValid(), expected);

  for (const auto& index : blockIndex.pnext) {
    validityFlagCheck(*index, expected);
  }
}

/**
 * Start at initial state (bootstrapped with genesis blocks in regtest for all
 * chains).
 *
 * 1. Mine 500 VBK blocks
 * 2. Mine 10 ALT blocks
 * 3. Endorse VBK blocks 90 (VTB2) and 490(VTB1).
 * 4. Endorse ALT block 5 twice (ATV1, ATV2)
 * 5. Payloads1={ATV1, VTB1}, Payloads2={ATV2, VTB2}
 * 6. Send Payloads1, containing block = ALT11
 * 7. Send Payloads2, containing block = ALT12
 *
 * Expect Payloads1 is valid, accepted by ALT and state changed.
 * Expect Payloads2 is invalid. Payloads added, but state is not changed.
 */
TEST_F(Scenario8, scenario_8) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer->vbk().getParams());

  // mine 500 vbk blocks
  auto* vbkTip = popminer->mineVbkBlocks(
      vbkparam.getEndorsementSettlementInterval() + 100);

  // endorse block 490
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  // endorse block 90
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(
      vbkTip->height - 10 - vbkparam.getEndorsementSettlementInterval());

  VbkPopTx popTx1 = generatePopTx(*endorsedVbkBlock1->header);
  // remove this popTx from the mempool, so vbk blocks can mine correctly
  ASSERT_EQ(popminer->vbkmempool.size(), 1);
  popminer->vbkmempool.erase(popminer->vbkmempool.begin());

  VbkPopTx popTx2 = generatePopTx(*endorsedVbkBlock2->header);
  // remove this popTx from the mempool, so vbk blocks can mine correctly
  ASSERT_EQ(popminer->vbkmempool.size(), 1);
  popminer->vbkmempool.erase(popminer->vbkmempool.begin());

  // generate invalid VTB
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({popTx1, popTx2});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer->vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // Create VTV
  VTB vtb1;
  vtb1.transaction = popTx1;
  vtb1.merklePath.treeIndex = treeIndex;
  vtb1.merklePath.index = 0;
  vtb1.merklePath.subject = hashes[0];
  vtb1.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb1.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb1, state, popminer->vbk().getParams(), popminer->btc().getParams()));

  // Create VTV
  VTB vtb2;
  vtb2.transaction = popTx2;
  vtb2.merklePath.treeIndex = treeIndex;
  vtb2.merklePath.index = 1;
  vtb2.merklePath.subject = hashes[1];
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[1]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer->vbk().getParams(), popminer->btc().getParams()));

  EXPECT_TRUE(popminer->vbk().acceptBlock(containingVbkBlock, state));

  fillVbkContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer->vbk());
  fillVbkContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer->vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];
  VbkTx tx1 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv1 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  PopData popData1 = createPopData(0, {atv1}, {vtb1});

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, popData1, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(*alttree.vbk().getBestChain().tip(),
            *popminer->vbk().getBestChain().tip());

  auto altStateVbkTip = *alttree.vbk().getBestChain().tip();

  auto* vbkBlock = alttree.vbk().getBlockIndex(containingVbkBlock.getHash());
  EXPECT_NE(vbkBlock, nullptr);
  validityFlagCheck(*vbkBlock, true);

  VbkTx tx2 = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  ATV atv2 =
      popminer->generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  PopData popData2 = createPopData(0, {atv2}, {vtb2});

  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, popData2, state));
  EXPECT_FALSE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetDebugMessage(), "Endorsement expired");

  // VBK subtree 501 (contains expired VTB) is VALID - because we tried to add
  // VTB, it was invalid, so we immediately removed it
  validityFlagCheck(*vbkBlock, true);
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->height, 502);

  vbkBlock = alttree.vbk().getBlockIndex(containingVbkBlock.getHash());
  ASSERT_NE(vbkBlock, nullptr);

  // remove payloads from alt, vbk state is still valid
  alttree.removePayloads(containingBlock.hash, popData2);
  ASSERT_TRUE(alttree.setState(containingBlock.hash, state));
  validityFlagCheck(*vbkBlock, true);
}
