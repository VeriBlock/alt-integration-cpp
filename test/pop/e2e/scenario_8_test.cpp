// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

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

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer.vbk().getParams());

  // mine 500 vbk blocks
  auto* vbkTip =
      popminer.mineVbkBlocks(vbkparam.getEndorsementSettlementInterval() + 100);

  // endorse block 490
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->getHeight() - 10);
  // endorse block 90
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(
      vbkTip->getHeight() - 10 - vbkparam.getEndorsementSettlementInterval());

  VbkPopTx popTx1 = generatePopTx(endorsedVbkBlock1->getHeader());
  VbkPopTx popTx2 = generatePopTx(endorsedVbkBlock2->getHeader());

  // generate invalid VTB
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({popTx1, popTx2});
  VbkMerkleTree mtree({}, hashes);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer.vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // Create VTB
  VTB vtb1;
  vtb1.transaction = popTx1;
  vtb1.merklePath.treeIndex = (int32_t)VbkMerkleTree::TreeIndex::POP;
  vtb1.merklePath.index = 0;
  vtb1.merklePath.subject = hashes[0];
  vtb1.merklePath.layers =
      mtree.getMerklePathLayers(0, VbkMerkleTree::TreeIndex::POP);
  vtb1.containingBlock = containingVbkBlock;

  ASSERT_TRUE(checkVTB(
      vtb1, state, popminer.btc().getParams(), popminer.vbk().getParams()))
      << state.toString();

  // Create VTB
  VTB vtb2;
  vtb2.transaction = popTx2;
  vtb2.merklePath.treeIndex =
      static_cast<int32_t>(VbkMerkleTree::TreeIndex::POP);
  vtb2.merklePath.index = 1;
  vtb2.merklePath.subject = hashes[1];
  vtb2.merklePath.layers =
      mtree.getMerklePathLayers(1, VbkMerkleTree::TreeIndex::POP);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer.btc().getParams(), popminer.vbk().getParams()));

  EXPECT_TRUE(popminer.vbk().acceptBlockHeader(containingVbkBlock, state));

  // mine 10 blocks
  mineAltBlocks(10, chain, /*connectBlocks=*/true, /*setState=*/false);

  AltBlock endorsedBlock = chain[5];
  VbkTx tx1 = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block1 = popminer.mineVbkBlocks(1, {tx1});
  ATV atv1 = popminer.createATV(block1->getHeader(), tx1);

  PopData popData1;
  popData1.atvs = {atv1};
  popData1.vtbs = {vtb1};

  fillVbkContext(
      popData1.context, GetRegTestVbkBlock().getHash(), popminer.vbk());

  auto containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  // add alt payloads
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData1));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());
  validateAlttreeIndexState(alttree, containingBlock, popData1);

  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHash(),
            popminer.vbk().getBestChain().tip()->getHash());

  auto* vbkBlock = alttree.vbk().getBlockIndex(containingVbkBlock.getHash());
  EXPECT_NE(vbkBlock, nullptr);
  validityFlagCheck(*vbkBlock, true);

  VbkTx tx2 = popminer.createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  auto* block2 = popminer.mineVbkBlocks(1, {tx2});
  ATV atv2 = popminer.createATV(block2->getHeader(), tx2);

  PopData popData2;
  popData2.atvs = {atv2};
  popData2.vtbs = {vtb2};

  fillVbkContext(
      popData2.context, GetRegTestVbkBlock().getHash(), popminer.vbk());

  containingBlock = generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  popData2.context.clear();
  EXPECT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
  EXPECT_TRUE(AddPayloads(containingBlock.getHash(), popData2));
  EXPECT_FALSE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetPath(),
            "ALT-bad-command+VBK-invalid-payloads+VBK-expired");
  // all payloads are marked valid as there's no correctly implemented
  // invalidation
  validateAlttreeIndexState(
      alttree, containingBlock, popData2, /*payloads_validation =*/true);

  // VBK subtree 501 (contains expired VTB) is VALID - because we tried to add
  // VTB, it was invalid, so we immediately removed it
  validityFlagCheck(*vbkBlock, true);
  EXPECT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 502);

  vbkBlock = alttree.vbk().getBlockIndex(containingVbkBlock.getHash());
  ASSERT_NE(vbkBlock, nullptr);

  // remove payloads from alt, vbk state is still valid
  auto* containingIndex = alttree.getBlockIndex(containingBlock.getHash());
  ASSERT_TRUE(containingIndex);
  alttree.removePayloads(containingIndex->getHash());
  validityFlagCheck(*vbkBlock, true);
  ASSERT_TRUE(SetState(alttree, containingBlock.previousBlock));
  validityFlagCheck(*vbkBlock, true);

  ASSERT_DEATH(alttree.setState(containingBlock.getHash(), state),
               "must be connected");
}
