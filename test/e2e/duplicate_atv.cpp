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
  payloads.containingBlock = chain[100];
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {payloads}, state));
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate");
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_BA) {
  payloads.containingBlock = chain[100];
  ASSERT_TRUE(alttree.addPayloads(chain[100].hash, {payloads}, state));
  payloads.containingBlock = chain[99];
  ASSERT_TRUE(alttree.addPayloads(chain[99].hash, {payloads}, state));
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate");
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_AA) {
  payloads.containingBlock = chain[100];
  ASSERT_TRUE(alttree.validatePayloads(chain[100].hash, payloads, state));
  ASSERT_FALSE(alttree.validatePayloads(chain[100].hash, payloads, state));
  ASSERT_EQ(state.GetPath(),
            "ALT-addPayloadsTemporarily+ALT-bad-command+VBK-duplicate");
}

TEST_F(DuplicateATVfixture, DuplicateATV_DifferentContaining_2A) {
  payloads.containingBlock = chain[100];
  ASSERT_TRUE(
      alttree.addPayloads(chain[100].hash, {payloads, payloads}, state));
  ASSERT_FALSE(alttree.setState(chain[100].hash, state));
  ASSERT_EQ(state.GetPath(), "ALT-bad-command+VBK-duplicate");
}