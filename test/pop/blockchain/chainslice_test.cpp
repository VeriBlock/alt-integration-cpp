// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <exception>

#include "pop/util/pop_test_fixture.hpp"

using namespace altintegration;

struct ChainSliceTestFixture : public ::testing::Test, public PopTestFixture {
  using block_t = VbkBlock;
  using index_t = BlockIndex<block_t>;
  using height_t = block_t::height_t;
  using slice_t = ChainSlice<index_t>;
  using chain_t = Chain<index_t>;

  chain_t chain;
  const height_t forkHeight = 10;
  index_t *forkPoint;
  index_t *forkTip;
  chain_t fork;

  ChainSliceTestFixture() {
    popminer->mineVbkBlocks(30);

    forkPoint = popminer->vbk().getBestChain().tip()->getAncestor(forkHeight);
    forkTip = popminer->mineVbkBlocks(10, *forkPoint);
    chain = Chain(5, popminer->vbk().getBestChain().tip()->getAncestor(25));
    fork = Chain(forkHeight, forkTip);
  }
};

TEST_F(ChainSliceTestFixture, EmptySlice) {
  slice_t slice{chain, chain.firstHeight() + 1, 0};

  ASSERT_EQ(slice.size(), 0);
  ASSERT_TRUE(slice.empty());

  ASSERT_EQ(slice.first(), nullptr);
  ASSERT_EQ(slice.tip(), nullptr);
}

TEST_F(ChainSliceTestFixture, CopySliceSize) {
  slice_t slice(chain);

  ASSERT_EQ(chain.firstHeight(), slice.firstHeight());
  ASSERT_EQ(chain.size(), slice.size());

  ASSERT_NE(slice.first(), nullptr);
  ASSERT_NE(slice.tip(), nullptr);

  ASSERT_EQ(chain.first(), slice.first());
  ASSERT_EQ(chain.tip(), slice.tip());
}

TEST_F(ChainSliceTestFixture, OffsetSliceSize) {
  auto const offset = 2;
  slice_t slice(chain, chain.firstHeight() + offset);

  ASSERT_EQ(chain.firstHeight() + offset, slice.firstHeight());
  ASSERT_EQ(chain.size(), slice.size() + offset);

  ASSERT_NE(slice.first(), nullptr);
  ASSERT_NE(slice.tip(), nullptr);

  ASSERT_EQ(chain[chain.firstHeight() + offset], slice.first());
  ASSERT_EQ(chain.tip(), slice.tip());
}

TEST_F(ChainSliceTestFixture, SubseSliceSize) {
  auto const offset = 5;
  auto const size = 7;

  slice_t slice(chain, chain.firstHeight() + offset, size);

  ASSERT_EQ(chain.firstHeight() + offset, slice.firstHeight());

  ASSERT_NE(slice.first(), nullptr);
  ASSERT_NE(slice.tip(), nullptr);

  ASSERT_EQ(chain[chain.firstHeight() + offset], slice.first());
  ASSERT_EQ(chain[chain.firstHeight() + offset + size - 1], slice.tip());
}

TEST_F(ChainSliceTestFixture, SliceIsWithinChain) {
  ASSERT_DEATH(ChainSlice(chain, chain.firstHeight() - 1, chain.size()), "");
  ASSERT_DEATH(ChainSlice(chain, chain.firstHeight() + 1, chain.size()), "");
}

TEST_F(ChainSliceTestFixture, SliceBlocks) {
  auto const first = 7;
  auto const size = 2;

  slice_t slice{chain, first, size};

  // the block preceding the first one is not in the slice
  ASSERT_NE(chain[first - 1], nullptr);
  ASSERT_EQ(slice[first - 1], nullptr);
  ASSERT_FALSE(slice.contains(chain[first - 1]));

  // the first block is accessible
  ASSERT_NE(chain[first], nullptr);
  ASSERT_EQ(slice[first], chain[first]);
  ASSERT_TRUE(slice.contains(chain[first]));

  // the last block is accessible
  ASSERT_NE(chain[first + size - 1], nullptr);
  ASSERT_EQ(slice[first + size - 1], chain[first + size - 1]);
  ASSERT_TRUE(slice.contains(chain[first + size - 1]));

  // the block after the last one is not in the slice
  ASSERT_NE(chain[first + size], nullptr);
  ASSERT_EQ(slice[first + size], nullptr);
  ASSERT_FALSE(slice.contains(chain[first + size]));
}

TEST_F(ChainSliceTestFixture, SliceIterators) {
  const auto offset = 5;
  const auto size = 10;

  slice_t slice{chain, chain.firstHeight() + offset, size};
  chain_t equivalentChain{slice.firstHeight(), slice.tip()};

  // the slice is a subset of the undelying chain
  ASSERT_FALSE(
      std::equal(chain.begin(), chain.end(), slice.begin(), slice.end()));
  ASSERT_FALSE(
      std::equal(chain.rbegin(), chain.rend(), slice.rbegin(), slice.rend()));

  ASSERT_TRUE(std::equal(equivalentChain.begin(),
                         equivalentChain.end(),
                         slice.begin(),
                         slice.end()));
  ASSERT_TRUE(std::equal(equivalentChain.rbegin(),
                         equivalentChain.rend(),
                         slice.rbegin(),
                         slice.rend()));
}

TEST_F(ChainSliceTestFixture, SliceFork) {
  // the slice includes the fork point
  ASSERT_EQ(findFork(ChainSlice{chain, forkHeight - 2, 5}, forkTip), forkPoint);

  // the slice is a descendant of the fork point
  ASSERT_EQ(findFork(ChainSlice{chain, forkHeight + 2, 5}, forkTip), nullptr);

  // the slice is an encestor of the fork point
  ASSERT_EQ(findFork(ChainSlice{chain, forkHeight - 5, 2}, forkTip),
            chain[forkHeight - 5 + 1]);
}
