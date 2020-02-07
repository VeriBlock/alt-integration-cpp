#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <veriblock/blockchain/chain.hpp>

using namespace VeriBlock;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

struct DummyBlock {
  using hash_t = int;
  using height_t = int;
};

struct ChainTest : public ::testing::Test {
  Chain<DummyBlock> chain{};
  std::vector<BlockIndex<DummyBlock>> blocks{};
  const size_t SIZE = 10;

  ChainTest() {
    // prepare block storage
    for (size_t i = 0; i < SIZE; i++) {
      BlockIndex<DummyBlock> index{};
      index.height = (int)i;
      index.pprev = nullptr;
      blocks.push_back(index);
    }

    // no need to setup links for lists with less than 2 elements
    if (SIZE < 2) return;

    // fill in the links to previous blocks
    for (size_t i = 0; i < (SIZE - 1); i++) {
      auto elem = &blocks[SIZE - i - 1];
      elem->pprev = &blocks[SIZE - i - 2];
    }
  }

  // validate that chain contains the same blocks in the same order
  void validateChain() {
    auto* tip = chain.tip();
    ASSERT_NE(tip, nullptr);

    for (int i = 0; i < chain.size(); i++) {
      EXPECT_EQ(chain[i], &blocks[i]);
    }
  }
};

TEST_F(ChainTest, StartsAt0) {
  chain.setTip(&(*blocks.rbegin()));
  EXPECT_EQ(chain.size(), SIZE);
  EXPECT_EQ(chain.tip(), &(*blocks.rbegin()));

  // check 'contains' method
  for (int i = 0; i < (int)SIZE; i++) {
    EXPECT_EQ(chain[i], &blocks[i]);
    EXPECT_TRUE(chain.contains(&blocks[i]));
  }

  // check out of range blocks
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[(int)SIZE + 1], nullptr);

  // check 'next' method
  for (size_t i = 0; i < SIZE - 1; i++) {
    EXPECT_EQ(chain.next(&blocks[i]), &(blocks[i + 1]));
  }

  // check 'setTip' method
  for (size_t i = 0; i < SIZE; i++) {
    chain.setTip(&blocks[i]);
    validateChain();
  }
}