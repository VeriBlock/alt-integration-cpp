// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>

using namespace altintegration;

struct FinalizedPayloadsIndexTest : public ::testing::Test,
                                    public PopTestFixture {
  size_t totalBlocks = 0;
  size_t deallocatedAlt = 0;
  size_t deallocatedVbk = 0;
  size_t deallocatedBtc = 0;

  void SetUp() override {
    altparam.mMaxReorgBlocks = 60;
    altparam.mEndorsementSettlementInterval = 50;
    altparam.mPreserveBlocksBehindFinal = 50;

    alttree.onBlockBeforeDeallocated.connect(
        [&](const BlockIndex<AltBlock>&) { deallocatedAlt++; });

    alttree.vbk().onBlockBeforeDeallocated.connect(
        [&](const BlockIndex<VbkBlock>&) { deallocatedVbk++; });

    alttree.btc().onBlockBeforeDeallocated.connect(
        [&](const BlockIndex<BtcBlock>&) { deallocatedBtc++; });
  }

  void assertNonFinalizedPayloadsDoNotExistInFinalizedPayloadsIndex() {
    // all payloads that exist in a tree in all blocks except finalized should
    // NOT exist in finalized payloads index
    {  // check ALTTREE finalized payloads index
      for (const auto& id :
           getAllPayloadIdsInTree<ATV>(alttree, /*skipFinal=*/true)) {
        ASSERT_EQ(alttree.getFinalizedPayloadsIndex().find(id.asVector()),
                  nullptr);
      }
      for (const auto& id :
           getAllPayloadIdsInTree<VTB>(alttree, /*skipFinal=*/true)) {
        ASSERT_EQ(alttree.getFinalizedPayloadsIndex().find(id.asVector()),
                  nullptr);
      }
      for (const auto& id :
           getAllPayloadIdsInTree<VbkBlock>(alttree, /*skipFinal=*/true)) {
        ASSERT_EQ(alttree.getFinalizedPayloadsIndex().find(id.asVector()),
                  nullptr);
      }
    }

    // check VBK finalized payloads index
    for (const auto& id :
         getAllPayloadIdsInTree<VTB>(alttree.vbk(), /*skipFinal=*/true)) {
      EXPECT_EQ(alttree.vbk().getFinalizedPayloadsIndex().find(id.asVector()),
                nullptr);
    }
  }
};

TEST_F(FinalizedPayloadsIndexTest, FPIndexIsUpdatedAfterFinalization1) {
  createEndorsedAltChain(1500);
  save(alttree);

  auto allAtvs = getAllPayloadIdsInTree<ATV>(alttree);
  auto allVtbs = getAllPayloadIdsInTree<VTB>(alttree);
  auto allVbks = getAllPayloadIdsInTree<VbkBlock>(alttree);
  auto allVtbsInVbk = getAllPayloadIdsInTree<VTB>(alttree.vbk());

  // we didn't save blocks on disk, so we can not deallocate them
  ASSERT_EQ(deallocatedAlt, 0);
  ASSERT_EQ(deallocatedVbk, 0);
  ASSERT_EQ(deallocatedBtc, 0);
  ASSERT_TRUE(alttree.getFinalizedPayloadsIndex().empty());
  ASSERT_TRUE(alttree.vbk().getFinalizedPayloadsIndex().empty());
  ASSERT_TRUE(alttree.btc().getFinalizedPayloadsIndex().empty());
  ASSERT_GT(allAtvs.size(), 0);
  ASSERT_GT(allVtbs.size(), 0);
  ASSERT_GT(allVbks.size(), 0);
  ASSERT_GT(allVtbsInVbk.size(), 0);

  // after trees are saved, try to free some RAM
  alttree.finalizeBlocks();

  assertNonFinalizedPayloadsDoNotExistInFinalizedPayloadsIndex();

  // now, after "save" is called we check that we freed some RAM
  ASSERT_GT(deallocatedAlt, 0);
  ASSERT_GT(deallocatedVbk, 0);
  ASSERT_GT(deallocatedBtc, 0);
  ASSERT_FALSE(alttree.getFinalizedPayloadsIndex().empty());
  ASSERT_FALSE(alttree.vbk().getFinalizedPayloadsIndex().empty());
  // we have no payloads in BTC blocks, so this one will always be empty
  ASSERT_TRUE(alttree.btc().getFinalizedPayloadsIndex().empty());
}
