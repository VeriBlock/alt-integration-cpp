// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct DuplicateATVfixture : public ::testing::Test, public PopTestFixture {
  std::vector<AltBlock> chain;

  PopData payloads;
  AltBlock endorsed;
  AltBlock containing;

  ValidationState state;

  DuplicateATVfixture() {
    chain.push_back(alttree.getParams().getBootstrapBlock());
    mineAltBlocks(100, chain);
    endorsed = chain[50];
    containing = chain[100];
    payloads = endorseAltBlock({endorsed}, 10);
  }
};

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_AB) {
  ASSERT_TRUE(alttree.addPayloads(chain[99].getHash(), payloads, state));
  ASSERT_TRUE(alttree.setState(chain[99].getHash(), state));
  ASSERT_TRUE(alttree.addPayloads(chain[100].getHash(), payloads, state));
  ASSERT_FALSE(alttree.setState(chain[100].getHash(), state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");

  // we are at chain[99]
  ASSERT_EQ(alttree.getBestChain().tip()->getHeader(), chain[99]);

  // chain100 exists and marked as invalid
  auto index100 = alttree.getBlockIndex(chain[100].getHash());
  ASSERT_TRUE(index100);
  ASSERT_FALSE(index100->isValid());
  // chain100 contains 1 command group
  auto& atvids = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids.size(), 1);
  // with id == payloads.id
  ASSERT_EQ(*atvids.begin(), payloads.atvs[0].getId());

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(
      alttree.removePayloads(chain[100].getHash(), payloads));
  // index100 is now valid
  ASSERT_TRUE(index100->isValid());
  // chain100 contains 0 command groups
  auto& atvids2 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids2.size(), 0);
  // we can switch back to chain 100
  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_BA_removeA) {
  auto p1 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[100].getHash(), p1, state));
  auto p1id = payloads.atvs[0].getId();

  auto p2 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[99].getHash(), p2, state));
  auto p2id = payloads.atvs[0].getId();

  auto index99 = alttree.getBlockIndex(chain[99].getHash());
  auto& atvids = index99->getPayloadIds<ATV>();
  ASSERT_EQ(atvids.size(), 1);
  ASSERT_EQ(*atvids.begin(), p2id);
  ASSERT_TRUE(index99->isValid());
  auto index100 = alttree.getBlockIndex(chain[100].getHash());
  ASSERT_TRUE(index100->isValid());
  auto& atvids2 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids2.size(), 1);
  ASSERT_EQ(*atvids2.begin(), p1id);

  // we should be able to switch to 99
  ASSERT_TRUE(alttree.setState(chain[99].getHash(), state));
  // but not to 100, because 100 duplicates 99
  ASSERT_FALSE(alttree.setState(chain[100].getHash(), state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");
  auto& atvids3 = index99->getPayloadIds<ATV>();
  ASSERT_EQ(atvids3.size(), 1);
  ASSERT_EQ(*atvids3.begin(), p2id);

  ASSERT_TRUE(index99->isValid());
  ASSERT_FALSE(index100->isValid());
  auto& atvids4 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids4.size(), 1);
  ASSERT_EQ(*atvids4.begin(), p1id);

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(alttree.removePayloads(chain[100].getHash(), p1));
  // index100 is now valid
  ASSERT_TRUE(index100->isValid());
  auto& atvids5 = index100->getPayloadIds<ATV>();
  // chain100 contains 0 command groups
  ASSERT_EQ(atvids5.size(), 0);
  // we can switch back to chain 100
  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_BA_removeB) {
  auto p2 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[99].getHash(), p2, state));
  ASSERT_TRUE(alttree.setState(chain[99].getHash(), state));
  auto p2id = payloads.atvs.at(0).getId();

  auto p1 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[100].getHash(), p1, state));

  auto index99 = alttree.getBlockIndex(chain[99].getHash());
  auto& atvids = index99->getPayloadIds<ATV>();
  ASSERT_EQ(atvids.size(), 1);
  ASSERT_EQ(*atvids.begin(), p2.atvs.at(0).getId());
  ASSERT_TRUE(index99->isValid());
  auto index100 = alttree.getBlockIndex(chain[100].getHash());
  ASSERT_TRUE(index100->isValid());
  auto& atvids2 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids2.size(), 1);
  ASSERT_EQ(*atvids2.begin(), p1.atvs.at(0).getId());

  // we should be able to switch to 99
  ASSERT_TRUE(alttree.setState(chain[99].getHash(), state));
  // but not to 100, because 100 duplicates 99
  ASSERT_FALSE(alttree.setState(chain[100].getHash(), state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");
  auto& atvids3 = index99->getPayloadIds<ATV>();
  ASSERT_EQ(atvids3.size(), 1);
  ASSERT_EQ(*atvids3.begin(), p2id);
  ASSERT_TRUE(index99->isValid());
  ASSERT_FALSE(index100->isValid());
  auto& atvids4 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids4.size(), 1);
  ASSERT_EQ(*atvids4.begin(), p1.atvs.at(0).getId());

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(alttree.removePayloads(chain[99].getHash(), p2));
  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
  // both indices are now valid
  ASSERT_TRUE(index99->isValid());
  ASSERT_TRUE(index100->isValid());
  auto& atvids5 = index99->getPayloadIds<ATV>();
  auto& atvids6 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids5.size(), 0);
  ASSERT_EQ(atvids6.size(), 1);
  ASSERT_TRUE(alttree.setState(chain[100].getHash(), state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_SameContaining_AA) {
  auto index100 = alttree.getBlockIndex(chain[100].getHash());
  ASSERT_TRUE(index100);

  ASSERT_TRUE(alttree.validatePayloads(chain[100].getHash(), payloads, state));
  auto& atvids = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids.size(), 1);
  ASSERT_TRUE(index100->isValid());

  ASSERT_FALSE(alttree.validatePayloads(chain[100].getHash(), payloads, state));
  ASSERT_EQ(state.GetPath(),
            "ALT-addPayloadsTemporarily+ALT-duplicate-payloads");
  ASSERT_TRUE(index100->isValid());
  auto& atvids2 = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids2.size(), 1);
  ASSERT_EQ(*atvids2.begin(), payloads.atvs.at(0).getId());
}

TEST_F(DuplicateATVfixture, DuplicateATV_SameContaining_2A) {
  payloads.atvs.push_back(payloads.atvs.at(0));

  ASSERT_FALSE(alttree.addPayloads(chain[100].getHash(), payloads, state));
  ASSERT_EQ(state.GetPath(), "ALT-duplicate-payloads");

  auto index100 = alttree.getBlockIndex(chain[100].getHash());
  ASSERT_TRUE(index100);
  auto& atvids = index100->getPayloadIds<ATV>();
  ASSERT_EQ(atvids.size(), 0);
  auto& vtbids = index100->getPayloadIds<VTB>();
  ASSERT_EQ(vtbids.size(), 0);
  auto& blockids = index100->getPayloadIds<VbkBlock>();
  ASSERT_EQ(blockids.size(), 0);
}
