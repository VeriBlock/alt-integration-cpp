// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/orphan_block_container.hpp"

#include <gtest/gtest.h>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct OrphanBlockContainerFixture : public ::testing::Test,
                                     public PopTestFixture {
  using orphan_container_t = OrphanBlockContainer<AltTree>;
  using block_with_data_t = orphan_container_t::block_with_data_t;
  using payloads_t = orphan_container_t::payloads_t;

  OrphanBlockContainer<AltTree> orphans;
  std::vector<AltBlock> chain;
  std::vector<AltBlock> fork;
  int forkPoint;
  std::shared_ptr<payloads_t> emptyPayloads = std::make_shared<payloads_t>();
  std::vector<std::shared_ptr<payloads_t>> chainPayloads = {{}};

  OrphanBlockContainerFixture() : orphans(alttree) {
    chain = {altparam.getBootstrapBlock()};
    for (size_t i = 0; i < 10; i++) {
      chainPayloads.push_back(
          std::make_shared<PopData>(endorseAltBlock({chain.back()}, 2)));
      chain.push_back(generateNextBlock(chain.back()));
    }

    forkPoint = 3;
    fork = {chain.begin(), chain.begin() + forkPoint + 1};
    for (size_t i = 0; i < 10; i++) {
      fork.push_back(generateNextBlock(fork.back()));
    }
  }

  void addHeaders(std::vector<AltBlock>& headers) {
    for (auto& header : headers) {
      ValidationState state;
      ASSERT_TRUE(orphans.acceptHeader(header, state)) << state.toString();
    }
  }
};

TEST_F(OrphanBlockContainerFixture, unknownBlocksAreRejected) {
  ValidationState state;
  ASSERT_EQ(orphans.size(), 0);

  // cannot add an unknown block
  ASSERT_FALSE(orphans.acceptBlock(chain[4], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 0);

  addHeaders(chain);

  // if the header is known and valid, the block is accepted
  ASSERT_TRUE(orphans.acceptBlock(chain[4], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 1);
}

TEST_F(OrphanBlockContainerFixture, addSubtreeOf5EmptyBlocks) {
  ValidationState state;

  addHeaders(chain);
  addHeaders(fork);
  ASSERT_EQ(orphans.size(), 0);

  // first block is added successfully
  ASSERT_FALSE(orphans.isOrphan(chain[1]));
  ASSERT_TRUE(orphans.acceptBlock(chain[1], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 0);
  orphans.assertConsistent();

  // add 3 blocks in the inverse order
  ASSERT_TRUE(orphans.isOrphan(chain[4]));
  ASSERT_TRUE(orphans.acceptBlock(chain[4], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 1);
  orphans.assertConsistent();

  ASSERT_TRUE(orphans.isOrphan(fork[4]));
  ASSERT_TRUE(orphans.acceptBlock(fork[4], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 2);
  orphans.assertConsistent();

  ASSERT_TRUE(orphans.isOrphan(chain[3]));
  ASSERT_TRUE(orphans.acceptBlock(chain[3], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 3);
  orphans.assertConsistent();

  // block 2 connects 3, 4 and forked 4 to the tree
  ASSERT_FALSE(orphans.isOrphan(chain[2]));
  ASSERT_TRUE(orphans.acceptBlock(chain[2], emptyPayloads, state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 0);
  orphans.assertConsistent();
}

TEST_F(OrphanBlockContainerFixture, addSubtreeWithInvalidPayloads) {
  ValidationState state;

  addHeaders(chain);
  addHeaders(fork);
  ASSERT_EQ(orphans.size(), 0);

  // first block is added successfully
  ASSERT_FALSE(orphans.isOrphan(chain[1]));
  ASSERT_TRUE(orphans.acceptBlock(chain[1], chainPayloads[1], state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 0);
  orphans.assertConsistent();

  // add 4 blocks in the inverse order
  ASSERT_TRUE(orphans.isOrphan(chain[4]));
  ASSERT_TRUE(orphans.acceptBlock(chain[4], chainPayloads[4], state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 1);
  orphans.assertConsistent();

  // broken payloads
  ASSERT_TRUE(orphans.isOrphan(fork[4]));
  ASSERT_TRUE(orphans.acceptBlock(fork[4], chainPayloads[8], state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 2);
  orphans.assertConsistent();

  // broken payloads
  ASSERT_TRUE(orphans.isOrphan(fork[5]));
  ASSERT_TRUE(orphans.acceptBlock(fork[5], chainPayloads[9], state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 3);
  orphans.assertConsistent();

  ASSERT_TRUE(orphans.isOrphan(chain[3]));
  ASSERT_TRUE(orphans.acceptBlock(chain[3], chainPayloads[3], state))
      << state.toString();
  ASSERT_EQ(orphans.size(), 4);
  orphans.assertConsistent();

  // block 2 connects 3, 4 and forked 4, 5 to the tree
  ASSERT_FALSE(orphans.isOrphan(chain[2]));
  ASSERT_TRUE(orphans.acceptBlock(chain[2], chainPayloads[2], state))
      << state.toString();

  ASSERT_EQ(orphans.size(), 0);
  orphans.assertConsistent();
}
