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

    for (int i = chain.getStartHeight(); i <= chain.height(); i++) {
      EXPECT_EQ(chain[i], &blocks[i]);
    }
  }
};

TEST_F(ChainTest, StartsAt0) {
  chain.setTip(&(*blocks.rbegin()));
  EXPECT_EQ(chain.height(), SIZE - 1);
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

TEST_F(ChainTest, StartsAt1) {
  // chain starts with height = 1
  chain = Chain<DummyBlock>(1);
  chain.setTip(&(*blocks.rbegin()));
  EXPECT_EQ(chain.height(), SIZE - 1);
  EXPECT_EQ(chain.tip(), &(*blocks.rbegin()));

  // check 'contains' method
  for (int i = 1; i < (int)SIZE; i++) {
    EXPECT_EQ(chain[i], &blocks[i]);
    EXPECT_TRUE(chain.contains(&blocks[i]));
  }
  // blocks with height = 0 are no longer in the chain
  EXPECT_EQ(chain[0], nullptr);
  EXPECT_FALSE(chain.contains(&blocks[0]));

  // check out of range blocks
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[(int)SIZE + 1], nullptr);

  // check 'next' method
  for (size_t i = 1; i < SIZE - 1; i++) {
    EXPECT_EQ(chain.next(&blocks[i]), &(blocks[i + 1]));
  }
  EXPECT_EQ(chain.next(&blocks[0]), nullptr);

  // check 'setTip' method
  for (size_t i = 1; i < SIZE; i++) {
    chain.setTip(&blocks[i]);
    validateChain();
  }
  // do not try to chain.setTip(&blocks[0]) - it breaks assertion
}
