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
  Chain<DummyBlock> chain;
  std::vector<BlockIndex<DummyBlock>> blocks;

  size_t SIZE = 10;

  ChainTest() {
    for (size_t i = 0; i < SIZE; i++) {
      BlockIndex<DummyBlock> index;
      index.height = i;
      index.pprev = i == 0 ? nullptr : &blocks[i - 1];
      blocks.push_back(index);
    }
  }

  void validateChain(Chain<DummyBlock>& c) {
    auto* tip = c.tip();
    ASSERT_NE(tip, nullptr);

    for (int i = 0; i < c.height(); i++) {
      EXPECT_EQ(c[i], &blocks[i]);
    }
  }
};

TEST_F(ChainTest, StartsAt0) {
  chain.setTip(&*blocks.rbegin());
  EXPECT_EQ(chain.height(), SIZE);
  EXPECT_EQ(chain.tip(), &(*blocks.rbegin()));
  for (size_t i = 0; i < SIZE; i++) {
    EXPECT_EQ(chain[i], &blocks[i]);
    EXPECT_TRUE(chain.contains(blocks[i]));
  }
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[SIZE + 1], nullptr);
  for (size_t i = 0; i < SIZE - 1; i++) {
    EXPECT_EQ(chain.next(blocks[i]), &blocks[i + 1]);
  }

  for (size_t i = 0; i < SIZE; i++) {
    chain.setTip(&blocks[i]);
    validateChain(chain);
  }
}