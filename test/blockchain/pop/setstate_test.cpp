// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct SetStateTest : public ::testing::Test, public PopTestFixture {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};

  void gen(int VTBs = 3) {
    mineAltBlocks(100, chain);
    const int ATVs = 1;
    ASSERT_TRUE(alttree.setState(chain[0].hash, state));
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
    auto e90c100 = endorseAltBlock({chain[90]}, VTBs);
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
    ASSERT_TRUE(alttree.getBlockIndex(chain[100].hash));
    ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {e90c100}, state));
    ASSERT_TRUE(alttree.setState(chain[100].hash, state));
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].hash);
    ASSERT_EQ(alttree.btc().getBestChain().tip()->height, VTBs);
    ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, VTBs + ATVs);
  }
};

TEST_F(SetStateTest, SetStateGenesis) {
  mineAltBlocks(1, chain);
  ASSERT_TRUE(alttree.setState(chain[1].hash, state));
  ASSERT_TRUE(alttree.setState(chain[0].hash, state));
}

TEST_F(SetStateTest, AddPayloadsInvalid) {
  mineAltBlocks(100, chain);
  const int VTBs = 1;
  ASSERT_TRUE(alttree.setState(chain[0].hash, state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
  auto e90c100 = endorseAltBlock({chain[90]}, VTBs);
  // break ATV
  e90c100.atvs.at(0).containingBlock.previousBlock = uint96();
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
  ASSERT_TRUE(alttree.getBlockIndex(chain[100].hash));
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {e90c100}, state));
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  // state has not been changed
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.getBlockIndex(chain[100].hash)->status,
            BLOCK_VALID_TREE | BLOCK_FAILED_POP);

  auto next = generateNextBlock(chain[100]);
  ValidationState state2;
  ASSERT_FALSE(alttree.acceptBlock(next, state2));
  ASSERT_EQ(state2.GetPath(), "ALT-bad-chain");
  auto index = alttree.getBlockIndex(next.getHash());
  ASSERT_TRUE(index);
  ASSERT_EQ(index->status, BLOCK_FAILED_CHILD);
}

TEST_F(SetStateTest, SetStateNextChainBlockNoPayloads) {
  mineAltBlocks(100, chain);
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].hash);
  ASSERT_TRUE(alttree.setState(chain[0].hash, state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].hash);
}

TEST_F(SetStateTest, AddPayloadsSingleChain) {
  const int VTBs = 3;
  const int ATVs = 1;
  gen(VTBs);

  ASSERT_TRUE(alttree.setState(chain[60].hash, state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[60].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.comparePopScore(chain[60].hash, chain[100].hash), -1);
  ASSERT_TRUE(alttree.setState(chain[60].hash, state)) << state.toString();
  // we applied block 60, but pass 100 as left fork. expect logic_error
  ASSERT_THROW(alttree.comparePopScore(chain[100].hash, chain[60].hash),
               std::logic_error);
  ASSERT_TRUE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(alttree.comparePopScore(chain[100].hash, chain[60].hash), 1);

  ASSERT_TRUE(alttree.setState(chain[100].hash, state)) << state.toString();
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, VTBs);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, VTBs + ATVs);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_RemoveTip) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(alttree.removeSubtree(chain[100].hash));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[99].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_InvalidateTip) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(
      alttree.invalidateSubtree(chain[100].hash, BLOCK_FAILED_BLOCK));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[99].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_RemoveSubchain) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(alttree.removeSubtree(chain[50].hash));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[49].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_InvalidateSubchain) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(
      alttree.invalidateSubtree(chain[50].hash, BLOCK_FAILED_BLOCK));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[49].hash);
  ASSERT_EQ(alttree.btc().getBestChain().tip()->height, 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->height, 0);
}
