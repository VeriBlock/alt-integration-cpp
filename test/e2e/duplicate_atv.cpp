// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct DuplicateATVfixture : public ::testing::Test, public PopTestFixture {
  std::vector<AltBlock> chain;

  AltPayloads payloads;
  AltBlock endorsed;
  AltBlock containing;

  ValidationState state;

  DuplicateATVfixture() {
    chain.push_back(alttree.getParams().getBootstrapBlock());
    mineAltBlocks(100, chain);
    endorsed = chain[50];
    containing = chain[100];
    payloads = endorseAltBlock(endorsed, containing, 10);
  }
};

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_AB) {
  payloads.containingBlock = chain[99];
  ASSERT_TRUE(alttree.addPayloads(chain[99].hash, {payloads}, state));
  ASSERT_TRUE(alttree.setState(chain[99].hash, state));
  payloads.containingBlock = chain[100];
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {payloads}, state));
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");

  // we are at chain[99]
  ASSERT_EQ(*alttree.getBestChain().tip()->header, chain[99]);

  // chain100 exists and marked as invalid
  auto index100 = alttree.getBlockIndex(chain[100].hash);
  ASSERT_TRUE(index100);
  ASSERT_FALSE(index100->isValid());
  // chain100 contains 1 command group
  ASSERT_EQ(index100->payloadIds.size(), 1);
  // with id == payloads.id
  ASSERT_EQ(*index100->payloadIds.begin(), payloads.getId());

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(alttree.removePayloads(chain[100].hash, {payloads.getId()}));
  // index100 is now valid
  ASSERT_TRUE(index100->isValid());
  // chain100 contains 0 command groups
  ASSERT_EQ(index100->payloadIds.size(), 0);
  // we can switch back to chain 100
  ASSERT_TRUE(alttree.setState(chain[100].hash, state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_BA_removeA) {
  payloads.containingBlock = chain[100];
  auto p1 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {p1}, state));
  auto p1id = payloads.getId();

  payloads.containingBlock = chain[99];
  auto p2 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[99].hash, {p2}, state));
  auto p2id = payloads.getId();

  auto index99 = alttree.getBlockIndex(chain[99].hash);
  ASSERT_EQ(index99->payloadIds.size(), 1);
  ASSERT_EQ(*index99->payloadIds.begin(), p2id);
  ASSERT_TRUE(index99->isValid());
  auto index100 = alttree.getBlockIndex(chain[100].hash);
  ASSERT_TRUE(index100->isValid());
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_EQ(*index100->payloadIds.begin(), p1id);

  // we should be able to switch to 99
  ASSERT_TRUE(alttree.setState(chain[99].hash, state));
  // but not to 100, because 100 duplicates 99
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");
  ASSERT_EQ(index99->payloadIds.size(), 1);
  ASSERT_EQ(*index99->payloadIds.begin(), p2id);

  ASSERT_TRUE(index99->isValid());
  ASSERT_FALSE(index100->isValid());
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_EQ(*index100->payloadIds.begin(), p1id);

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(alttree.removePayloads(chain[100].hash, {p1.getId()}));
  // index100 is now valid
  ASSERT_TRUE(index100->isValid());
  // chain100 contains 0 command groups
  ASSERT_EQ(index100->payloadIds.size(), 0);
  // we can switch back to chain 100
  ASSERT_TRUE(alttree.setState(chain[100].hash, state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_BA_removeB) {
  payloads.containingBlock = chain[99];
  auto p2 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[99].hash, {p2}, state));
  ASSERT_TRUE(alttree.setState(chain[99].hash, state));
  auto p2id = payloads.getId();

  payloads.containingBlock = chain[100];
  auto p1 = payloads;
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {p1}, state));

  auto index99 = alttree.getBlockIndex(chain[99].hash);
  ASSERT_EQ(index99->payloadIds.size(), 1);
  ASSERT_EQ(*index99->payloadIds.begin(), p2.getId());
  ASSERT_TRUE(index99->isValid());
  auto index100 = alttree.getBlockIndex(chain[100].hash);
  ASSERT_TRUE(index100->isValid());
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_EQ(*index100->payloadIds.begin(), p1.getId());

  // we should be able to switch to 99
  ASSERT_TRUE(alttree.setState(chain[99].hash, state));
  // but not to 100, because 100 duplicates 99
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate-payloads");
  ASSERT_EQ(index99->payloadIds.size(), 1);
  ASSERT_EQ(*index99->payloadIds.begin(), p2id);
  ASSERT_TRUE(index99->isValid());
  ASSERT_FALSE(index100->isValid());
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_EQ(*index100->payloadIds.begin(), p1.getId());

  // now we remove that duplicating payloads
  ASSERT_NO_FATAL_FAILURE(alttree.removePayloads(chain[99].hash, {p2.getId()}));
  ASSERT_TRUE(alttree.setState(chain[100].hash, state));
  // both indices are now valid
  ASSERT_TRUE(index99->isValid());
  ASSERT_TRUE(index100->isValid());
  ASSERT_EQ(index99->payloadIds.size(), 0);
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_TRUE(alttree.setState(chain[100].hash, state));
}

TEST_F(DuplicateATVfixture, DuplicateATV_SameContaining_AA) {
  payloads.containingBlock = chain[100];
  auto index100 = alttree.getBlockIndex(chain[100].hash);
  ASSERT_TRUE(index100);

  ASSERT_TRUE(alttree.validatePayloads(chain[100].hash, payloads, state));
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_TRUE(index100->isValid());

  ASSERT_FALSE(alttree.validatePayloads(chain[100].hash, payloads, state));
  ASSERT_EQ(state.GetPath(),
            "ALT-addPayloadsTemporarily+ALT-duplicate-payloads");
  ASSERT_EQ(index100->payloadIds.size(), 1);
  ASSERT_TRUE(index100->isValid());
  ASSERT_EQ(*index100->payloadIds.begin(), payloads.getId());
}

TEST_F(DuplicateATVfixture, DuplicateATV_SameContaining_2A) {
  payloads.containingBlock = chain[100];
  ASSERT_FALSE(
      alttree.addPayloads(chain[100].hash, {payloads, payloads}, state));
  ASSERT_EQ(state.GetPath(), "ALT-duplicate-payloads");

  auto index100 = alttree.getBlockIndex(chain[100].hash);
  ASSERT_TRUE(index100);
  ASSERT_EQ(index100->payloadIds.size(), 1);
}