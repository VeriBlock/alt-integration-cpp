// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <mempool_fixture.hpp>
#include <util/pop_test_fixture.hpp>

#include "veriblock/pop/validation_state.hpp"

using namespace altintegration;

struct StatefulDuplicateWhenFinalized : public ::testing::Test,
                                        public PopTestFixture {
  void SetUp() override {
    altparam.mMaxReorgDistance = 100;
    altparam.mEndorsementSettlementInterval = 50;
    altparam.mPreserveBlocksBehindFinal = 50;
  }

  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};
};

TEST_F(StatefulDuplicateWhenFinalized, VBK) {
  alttree.onInvalidBlockConnected.connect(
      [](BlockIndex<AltBlock> &/*index*/, ValidationState &state) {
        EXPECT_EQ(state.GetPath(), "VBK-duplicate");
      });

  auto *tip = alttree.getBestChain().tip();
  ASSERT_TRUE(tip);
  tip = mineAltBlocks(*tip, 10, PopData{}, /*setState=*/true);
  ASSERT_TRUE(tip);

  // prepare a PopData with 1 VBK block
  auto *vbk = popminer->mineVbkBlocks(1);
  PopData pd;
  pd.context.push_back(vbk->getHeader());

  save(alttree);

  // mine PopData with 1 VBK block in ALT:11
  tip = mineAltBlocks(*tip, 1, pd, /*setState=*/true);
  ASSERT_TRUE(tip);
  ASSERT_EQ(tip->getHeight(), 11);

  // mine mMaxReorgDistance-2 blocks
  tip = mineAltBlocks(
      *tip, altparam.mMaxReorgDistance - 2, PopData{}, /*setState=*/true);
  ASSERT_TRUE(tip);

  save(alttree);

  for (uint32_t i = 0; i < altparam.mPreserveBlocksBehindFinal + 4; i++) {
    // try to mine a duplicate in a block
    // altparam.mMaxReorgDistance{-1,0,+1,...}
    auto *invalid = mineAltBlocks(*tip, 1, pd, /*setState=*/false);
    ASSERT_FALSE(invalid->isValid())
        << "This block MUST be invalid, because it has stateful VBK duplicate";
    ASSERT_TRUE(state.IsValid());
    save(alttree);
    ASSERT_TRUE(tip->isValidTip());
    tip = mineAltBlocks(*tip, 1, PopData{}, /*setState=*/true);
    ASSERT_TRUE(tip->isValidTip());
    save(alttree);
  }

  // EXPECT_EQ(alttree.getBlocks().size(), 118);
  // ASSERT_EQ(alttree.getBestChain().tip()->getHeight(), 113);
}