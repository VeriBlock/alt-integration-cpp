#include "veriblock/mempool.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "util/pop_test_fixture.hpp"
#include "veriblock/hashutil.hpp"

using namespace altintegration;

std::vector<uint8_t> hash_function(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  AltBlock altBlock = AltBlock::fromVbkEncoding(stream);
  return altBlock.getHash();
}

struct MemPoolFixture : public PopTestFixture, public ::testing::Test {};

TEST_F(MemPoolFixture, removePayloads_test) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(*endorsedVbkBlock1->header);
  popminer.mineBtcBlocks(100);
  generatePopTx(*endorsedVbkBlock2->header);

  vbkTip = popminer.mineVbkBlocks(1);

  auto& vtbs = popminer.vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(BtcEndorsement::fromContainer(vtbs[0]).id,
            BtcEndorsement::fromContainer(vtbs[1]).id);
  fillVTBContext(vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));
  EXPECT_TRUE(mempool.submitVTB(vtbs, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 2);
  EXPECT_EQ(v_popData[0].atv, atv);

  // do the same to show that from mempool do not remove payloads
  v_popData = mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 2);
  EXPECT_EQ(v_popData[0].atv, atv);

  // remove from mempool
  mempool.removePayloads(v_popData);

  v_popData = mempool.getPop(*chain.rbegin(), alttree, state);
  EXPECT_EQ(v_popData.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_1) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(*endorsedVbkBlock1->header);
  popminer.mineBtcBlocks(100);
  generatePopTx(*endorsedVbkBlock2->header);

  vbkTip = popminer.mineVbkBlocks(1);

  auto& vtbs = popminer.vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  ASSERT_NE(BtcEndorsement::fromContainer(vtbs[0]).id,
            BtcEndorsement::fromContainer(vtbs[1]).id);
  fillVTBContext(vtbs[0], vbkparam.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(vtbs[1], vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));
  EXPECT_TRUE(mempool.submitVTB(vtbs, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 2);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MemPoolFixture, getPop_scenario_2) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer.vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVTBContext(vtb2, containingVbkBlock1->getHash(), popminer.vbk());

  ASSERT_NE(BtcEndorsement::fromContainer(vtb1).id,
            BtcEndorsement::fromContainer(vtb2).id);

  vbkTip = popminer.vbk().getBestChain().tip();

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb1, vtb2}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 2);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MemPoolFixture, getPop_scenario_3) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_4) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(
      tx, popminer.vbk().getParams().getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 0);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MemPoolFixture, getPop_scenario_5) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);

  generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer.generateATV(tx1, containingVbkBlock1->getHash(), state);

  vbkTip = popminer.vbk().getBestChain().tip();

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer.vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVTBContext(vtb2, vbkTip->getHash(), popminer.vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer.generateATV(tx2, containingVbkBlock2->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv1, atv2}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb2, vtb1}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 2);
  EXPECT_EQ(v_popData[0].vtbs.size(), 1);
  EXPECT_EQ(v_popData[0].atv, atv1);
  EXPECT_EQ(v_popData[0].vtbs[0], vtb1);
  EXPECT_EQ(v_popData[1].vtbs.size(), 1);
  EXPECT_EQ(v_popData[1].atv, atv2);
  EXPECT_EQ(v_popData[1].vtbs[0], vtb2);

  // TODO: uncomment affter fixing bug with the payloads
  /*
  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads1 =
      generateAltPayloads(popTxs[0], containingBlock, endorsedBlock1);
  AltPayloads payloads2 =
      generateAltPayloads(popTxs[1], containingBlock, endorsedBlock2);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(
      alttree.addPayloads(containingBlock, {payloads1, payloads2}, state));
  EXPECT_TRUE(state.IsValid());
  */
}

TEST_F(MemPoolFixture, getPop_scenario_6) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);

  generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer.generateATV(tx1, containingVbkBlock1->getHash(), state);

  vbkTip = popminer.vbk().getBestChain().tip();

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer.vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVTBContext(vtb2, vbkTip->getHash(), popminer.vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer.generateATV(tx2, containingVbkBlock2->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv2, atv1}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb2, vtb1}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 2);
  EXPECT_EQ(v_popData[0].vtbs.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs[0], vtb1);
  EXPECT_EQ(v_popData[0].atv, atv1);
  EXPECT_EQ(v_popData[1].vtbs.size(), 1);
  EXPECT_EQ(v_popData[1].atv, atv2);
  EXPECT_EQ(v_popData[1].vtbs[0], vtb2);

  // TODO: uncomment affter fixing bug with the payloads
  /*
  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads1 =
      generateAltPayloads(popTxs[0], containingBlock, endorsedBlock1);
  AltPayloads payloads2 =
      generateAltPayloads(popTxs[1], containingBlock, endorsedBlock2);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(
      alttree.addPayloads(containingBlock, {payloads1, payloads2}, state));
  EXPECT_TRUE(state.IsValid());
  */
}

TEST_F(MemPoolFixture, getPop_scenario_7) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  const auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);

  generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 = popminer.generateATV(tx1, containingVbkBlock1->getHash(), state);

  vbkTip = popminer.vbk().getBestChain().tip();

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  generatePopTx(*endorsedVbkBlock2->header);

  auto* containingVbkBlock2 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock2->getHash()].size(), 1);
  VTB vtb2 = popminer.vbkPayloads[containingVbkBlock2->getHash()][0];
  fillVTBContext(vtb2, containingVbkBlock2->pprev->getHash(), popminer.vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock2));
  ATV atv2 = popminer.generateATV(tx2, containingVbkBlock2->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv2, atv1}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb2, vtb1}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs[0], vtb1);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MemPoolFixture, getPop_scenario_8) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer.vbk().getParams());

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);

  auto vbkPopTx = generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  // generate VTB with the duplicate
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({vbkPopTx});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer.vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // add containing block without payloads into vbk tree
  EXPECT_TRUE(popminer.vbk().acceptBlock(containingVbkBlock, state));

  // Create VTV
  VTB vtb2;
  vtb2.transaction = vbkPopTx;
  vtb2.merklePath.treeIndex = treeIndex;
  vtb2.merklePath.index = 0;
  vtb2.merklePath.subject = hashes[0];
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer.vbk().getParams(), popminer.btc().getParams()));

  EXPECT_NE(vtb1, vtb2);
  EXPECT_EQ(BtcEndorsement::fromContainer(vtb1),
            BtcEndorsement::fromContainer(vtb2));

  fillVTBContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer.generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv1}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb1, vtb2}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs[0], vtb1);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MemPoolFixture, getPop_scenario_9) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer.vbk().getParams());

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer.generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv1}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].atv, atv1);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());

  mempool.removePayloads(v_popData);

  EXPECT_TRUE(mempool.submitATV({atv1}, state));

  v_popData = mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_10) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  Miner<VbkBlock, VbkChainParams> vbk_miner(popminer.vbk().getParams());

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  // endorse VBK blocks
  const auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);

  auto vbkPopTx = generatePopTx(*endorsedVbkBlock1->header);

  auto* containingVbkBlock1 = popminer.mineVbkBlocks(1);
  ASSERT_EQ(popminer.vbkPayloads[containingVbkBlock1->getHash()].size(), 1);
  VTB vtb1 = popminer.vbkPayloads[containingVbkBlock1->getHash()][0];
  fillVTBContext(vtb1, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  AltBlock endorsedBlock1 = chain[5];
  VbkTx tx1 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock1));
  ATV atv1 =
      popminer.generateATV(tx1, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv1}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb1}, state));

  std::vector<PopData> v_popData =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs[0], vtb1);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads1 =
      generateAltPayloads(v_popData[0], containingBlock, endorsedBlock1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads1}, state));
  EXPECT_TRUE(state.IsValid());

  // remove payloads from the mempool
  mempool.removePayloads(v_popData);

  popminer.mineBtcBlocks(100);
  popminer.mineVbkBlocks(54);

  // generate VTB with the duplicate
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>({vbkPopTx});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingVbkBlock = vbk_miner.createNextBlock(
      *popminer.vbk().getBestChain().tip(),
      mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // add containing block without payloads into vbk tree
  EXPECT_TRUE(popminer.vbk().acceptBlock(containingVbkBlock, state));

  // Create VTV
  VTB vtb2;
  vtb2.transaction = vbkPopTx;
  vtb2.merklePath.treeIndex = treeIndex;
  vtb2.merklePath.index = 0;
  vtb2.merklePath.subject = hashes[0];
  vtb2.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  vtb2.containingBlock = containingVbkBlock;

  EXPECT_TRUE(checkVTB(
      vtb2, state, popminer.vbk().getParams(), popminer.btc().getParams()));

  EXPECT_NE(vtb1, vtb2);
  EXPECT_EQ(BtcEndorsement::fromContainer(vtb1),
            BtcEndorsement::fromContainer(vtb2));

  fillVTBContext(vtb2, vbkparam.getGenesisBlock().getHash(), popminer.vbk());

  AltBlock endorsedBlock2 = chain[5];
  VbkTx tx2 = popminer.endorseAltBlock(generatePublicationData(endorsedBlock2));
  ATV atv2 =
      popminer.generateATV(tx2, vbkparam.getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv2}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb2}, state));

  v_popData = mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(v_popData.size(), 1);
  EXPECT_EQ(v_popData[0].vtbs.size(), 0);
}
