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
    ASSERT_TRUE(alttree.setState(chain[0].getHash(), state));
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
    auto e90c100 = endorseAltBlock({chain[90]}, VTBs);
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
    ASSERT_TRUE(alttree.getBlockIndex(chain[100].getHash()));
    ASSERT_TRUE(AddPayloads(chain[100].getHash(), {e90c100}));
    ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
    ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].getHash());
    ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), VTBs);
    ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), VTBs + ATVs);
  }
};

TEST_F(SetStateTest, SetStateGenesis) {
  mineAltBlocks(1, chain);
  ASSERT_TRUE(alttree.setState(chain[1].getHash(), state));
  ASSERT_TRUE(alttree.setState(chain[0].getHash(), state));
}

TEST_F(SetStateTest, AddPayloadsInvalid) {
  mineAltBlocks(100, chain);
  const int VTBs = 1;
  ASSERT_TRUE(alttree.setState(chain[0].getHash(), state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
  auto e90c100 = endorseAltBlock({chain[90]}, VTBs);
  // break ATV
  e90c100.atvs.at(0).blockOfProof.previousBlock = uint96();
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
  ASSERT_TRUE(alttree.getBlockIndex(chain[100].getHash()));
  ASSERT_TRUE(AddPayloads(chain[100].getHash(), {e90c100}));
  ASSERT_FALSE(alttree.setState(chain[100].getHash(), state));
  // state has not been changed
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
  ASSERT_TRUE(
      alttree.getBlockIndex(chain[100].getHash())->hasFlags(BLOCK_VALID_TREE));
  ASSERT_TRUE(
      alttree.getBlockIndex(chain[100].getHash())->hasFlags(BLOCK_FAILED_POP));

  auto next = generateNextBlock(chain[100]);
  ValidationState state2;
  ASSERT_FALSE(alttree.acceptBlockHeader(next, state2));
  ASSERT_EQ(state2.GetPath(), "ALT-bad-chain");
  auto index = alttree.getBlockIndex(next.getHash());
  ASSERT_TRUE(index);
  ASSERT_TRUE(index->hasFlags(BLOCK_FAILED_CHILD));
}

TEST_F(SetStateTest, SetStateNextChainBlockNoPayloads) {
  mineAltBlocks(100, chain);
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].getHash());
  ASSERT_TRUE(alttree.setState(chain[0].getHash(), state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[0].getHash());
}

TEST_F(SetStateTest, AddPayloadsSingleChain) {
  const int VTBs = 3;
  const int ATVs = 1;
  gen(VTBs);

  ASSERT_TRUE(alttree.setState(chain[60].getHash(), state));
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[60].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.comparePopScore(chain[60].getHash(), chain[100].getHash()),
            -1);
  ASSERT_TRUE(alttree.setState(chain[60].getHash(), state)) << state.toString();
  // we applied block 60, but pass 100 as left fork. expect logic_error
  ASSERT_DEATH(
      alttree.comparePopScore(chain[100].getHash(), chain[60].getHash()),
      "left fork must be applied");
  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
  ASSERT_EQ(alttree.comparePopScore(chain[100].getHash(), chain[60].getHash()),
            1);

  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state))
      << state.toString();
  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[100].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), VTBs);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), VTBs + ATVs);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_RemoveTip) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(alttree.removeSubtree(chain[100].getHash()));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[99].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_InvalidateTip) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(
      alttree.invalidateSubtree(chain[100].getHash(), BLOCK_FAILED_BLOCK));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[99].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_RemoveSubchain) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(alttree.removeSubtree(chain[50].getHash()));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[49].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
}

TEST_F(SetStateTest, AddPayloadsAtTip_then_InvalidateSubchain) {
  const int VTBs = 3;
  gen(VTBs);

  ASSERT_NO_FATAL_FAILURE(
      alttree.invalidateSubtree(chain[50].getHash(), BLOCK_FAILED_BLOCK));

  ASSERT_EQ(alttree.getBestChain().tip()->getHash(), chain[49].getHash());
  ASSERT_EQ(alttree.btc().getBestChain().tip()->getHeight(), 0);
  ASSERT_EQ(alttree.vbk().getBestChain().tip()->getHeight(), 0);
}
