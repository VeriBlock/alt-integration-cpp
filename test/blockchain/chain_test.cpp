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

  std::vector<BlockIndex<DummyBlock>> makeBlocks(int startHeight, int size) {
    std::vector<BlockIndex<DummyBlock>> blocks;
    for (int i = 0; i < size; i++) {
      BlockIndex<DummyBlock> index{};
      index.height = i + startHeight;
      index.pprev = nullptr;
      blocks.push_back(index);
    }

    // fill in the links to previous blocks
    for (int i = 0; i < (size - 1); i++) {
      auto elem = &blocks[size - i - 1];
      elem->pprev = &blocks[size - i - 2];
    }

    return blocks;
  }
};

TEST_F(ChainTest, StartsAt0) {
  auto START = 0;
  auto SIZE = 10;
  auto blocks = makeBlocks(START, SIZE);
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
  for (int i = 0; i < SIZE - 2; i++) {
    EXPECT_EQ(chain.next(&blocks[i]), &(blocks[i + 1]));
  }
  EXPECT_EQ(chain.next(&blocks[blocks.size() - 1]), nullptr);

  // check 'setTip' method
  for (int i = 0; i < SIZE; i++) {
    chain.setTip(&blocks[i]);
  }
}

TEST_F(ChainTest, StartsAt100) {
  auto SIZE = 10;
  auto START = 100;
  auto blocks = makeBlocks(START, SIZE);
  chain = Chain<DummyBlock>(START, &*blocks.rbegin());
  EXPECT_EQ(chain.height(), START + SIZE - 1);
  EXPECT_EQ(chain.tip(), &(*blocks.rbegin()));

  // check 'contains' method
  for (int i = 0; i < (int)SIZE; i++) {
    EXPECT_EQ(chain[i + START], &blocks[i]);
    EXPECT_TRUE(chain.contains(&blocks[i]));
  }
  // blocks with height = 0 are no longer in the chain
  EXPECT_EQ(chain[0], nullptr);

  // check out of range blocks
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[START + SIZE + 1], nullptr);

  // check 'next' method
  for (int i = 0; i < SIZE - 2; i++) {
    EXPECT_EQ(chain.next(&blocks[i]), &(blocks[i + 1]));
  }
  EXPECT_EQ(chain.next(&blocks[blocks.size() - 1]), nullptr);

  // check 'setTip' method
  for (int i = 0; i < SIZE; i++) {
    chain.setTip(&blocks[i]);
  }
}

TEST_F(ChainTest, CreateFrom0) {
  // when first block is at height 100 (no blocks behind that), and Chain is
  // created with height 0, it is expected to see that chain will contain 110
  // elements, first 100 of which are null.
  auto blocks = makeBlocks(100, 10);
  Chain<DummyBlock> c(0, &*blocks.rbegin());
  ASSERT_EQ(c.size(), 110);
  ASSERT_EQ(c.height(), 109);
}