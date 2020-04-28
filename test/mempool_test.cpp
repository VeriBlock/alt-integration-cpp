#include "veriblock/mempool.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "util/pop_test_fixture.hpp"
#include "veriblock/hashutil.hpp"

using namespace altintegration;

std::vector<uint8_t> hash_function(const std::vector<uint8_t>& bytes) {
  return sha256(bytes).asVector();
}

struct MemPoolFixture : public PopTestFixture, public ::testing::Test {};

TEST_F(MemPoolFixture, getPop_scenario_1) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  AltTree test_tree(alttree.getParams(),
                    alttree.vbk().getParams(),
                    alttree.btc().getParams());

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

  std::vector<AltPopTx> popTxs =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(popTxs.size(), 1);
  EXPECT_EQ(popTxs[0].vtbs.size(), 2);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(popTxs[0], containingBlock, endorsedBlock);

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
  AltTree test_tree(alttree.getParams(),
                    alttree.vbk().getParams(),
                    alttree.btc().getParams());

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

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));
  EXPECT_TRUE(mempool.submitVTB({vtb1, vtb2}, state));

  std::vector<AltPopTx> popTxs =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(popTxs.size(), 1);
  EXPECT_EQ(popTxs[0].vtbs.size(), 2);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(popTxs[0], containingBlock, endorsedBlock);

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
  AltTree test_tree(alttree.getParams(),
                    alttree.vbk().getParams(),
                    alttree.btc().getParams());

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(tx, vbkTip->getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));

  std::vector<AltPopTx> popTxs =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(popTxs.size(), 0);
}

TEST_F(MemPoolFixture, getPop_scenario_4) {
  MemPool mempool(alttree.getParams(),
                  alttree.vbk().getParams(),
                  alttree.btc().getParams(),
                  &hash_function);

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
  AltTree test_tree(alttree.getParams(),
                    alttree.vbk().getParams(),
                    alttree.btc().getParams());

  // mine 65 VBK blocks
  popminer.mineVbkBlocks(65);

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  ATV atv = popminer.generateATV(
      tx, popminer.vbk().getParams().getGenesisBlock().getHash(), state);

  EXPECT_TRUE(mempool.submitATV({atv}, state));

  std::vector<AltPopTx> popTxs =
      mempool.getPop(*chain.rbegin(), alttree, state);

  EXPECT_EQ(popTxs.size(), 1);
  EXPECT_EQ(popTxs[0].vtbs.size(), 0);

  auto containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads payloads =
      generateAltPayloads(popTxs[0], containingBlock, endorsedBlock);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, {payloads}, state));
  EXPECT_TRUE(state.IsValid());
}
