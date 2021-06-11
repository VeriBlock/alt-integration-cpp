// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <mempool_fixture.hpp>
#include <util/pop_test_fixture.hpp>

using namespace altintegration;

/**
 * Setup following tree:
 *                        /-E503
 *             / -B501-B502-B503
 * 0.....250..500-A501-A502-A503-A504 (tip)
 *        |    \ -C501-C502
 *        |          \-D502
 *        \-Z251
 *
 */
struct AltBlockRestoration : public ::testing::Test, public PopTestFixture {
  BlockIndex<AltBlock> *A504 = nullptr;
  BlockIndex<AltBlock> *B503 = nullptr;
  BlockIndex<AltBlock> *E503 = nullptr;
  BlockIndex<AltBlock> *C502 = nullptr;
  BlockIndex<AltBlock> *D502 = nullptr;
  BlockIndex<AltBlock> *Z251 = nullptr;

  size_t totalBlocks = 0;

  void SetUp() override {
    altparam.mMaxReorgDistance = 500;
    altparam.mEndorsementSettlementInterval = 50;
    altparam.mPreserveBlocksBehindFinal = 50;

    A504 = mineAltBlocks(alttree.getRoot(), 504);
    B503 = mineAltBlocks(*A504->getAncestor(500), 3);
    E503 = mineAltBlocks(*B503->getAncestor(502), 1);
    C502 = mineAltBlocks(*A504->getAncestor(500), 2);
    D502 = mineAltBlocks(*C502->getAncestor(501), 1);
    Z251 = mineAltBlocks(*A504->getAncestor(250), 1);

    ASSERT_TRUE(alttree.setState(*A504, state)) << state.toString();

    totalBlocks = alttree.getBlocks().size();
  }
};

TEST_F(AltBlockRestoration, RestoreFromActiveChainBasic) {
  // save state
  save(alttree);

  auto *tip = alttree.getBestChain().tip();
  auto *A50 = tip->getAncestor(50);
  auto *A200 = tip->getAncestor(200);
  const auto A50hash = A50->getHash();
  ASSERT_TRUE(alttree.finalizeBlock(*A200, state));
  // unchanged
  ASSERT_EQ(alttree.getBlocks().size(),
            totalBlocks - (200 - altparam.mPreserveBlocksBehindFinal));

  save(alttree);
  ASSERT_EQ(alttree.getBlockIndex(A50hash), nullptr);
  EXPECT_TRUE(alttree.restoreBlock(A50hash, state));
  ASSERT_NE(alttree.getBlockIndex(A50hash), nullptr);
  // 50 first blocks have not been restored
  ASSERT_EQ(alttree.getBlocks().size(), totalBlocks - 50);
}
