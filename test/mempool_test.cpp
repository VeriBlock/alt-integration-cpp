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

TEST_F(MemPoolFixture, getPop_basic) {
  MemPool mempool(&hash_function);

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

  EXPECT_TRUE(mempool.submitATV({atv}, alttree, state));
  EXPECT_TRUE(mempool.submitVTB(vtbs, alttree, state));

  AltPopTx popTx;
  EXPECT_TRUE(mempool.getPop(*chain.rbegin(), alttree, &popTx, state));
}
