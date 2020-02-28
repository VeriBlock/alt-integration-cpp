#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <veriblock/blockchain/chain.hpp>
#include <veriblock/fmt.hpp>

using namespace VeriBlock;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

struct DummyBlock {
  using hash_t = int;
  using height_t = int;
};

struct TestCase {
  int start;
  int size;
};

struct ChainTest : public ::testing::TestWithParam<TestCase> {
  Chain<DummyBlock> chain{};

  static std::vector<BlockIndex<DummyBlock>> makeBlocks(int startHeight,
                                                        int size) {
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

TEST_P(ChainTest, Full) {
  auto [start, size] = GetParam();
  auto blocks = makeBlocks(start, size);
  chain = Chain<DummyBlock>(start, &*blocks.rbegin());
  EXPECT_EQ(chain.height(), start + size - 1);
  EXPECT_EQ(chain.tip(), &(*blocks.rbegin()));

  // check 'contains' method
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(chain[i + start], &blocks[i]);
    EXPECT_TRUE(chain.contains(&blocks[i]));
  }
  EXPECT_EQ(chain[start - 1], nullptr);

  // check out of range blocks
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[start + size + 1], nullptr);

  // check 'next' method
  for (int i = 0; i < size - 2; i++) {
    EXPECT_EQ(chain.next(&blocks[i]), &(blocks[i + 1]));
  }
  EXPECT_EQ(chain.next(&blocks[blocks.size() - 1]), nullptr);

  // check 'setTip' method
  for (int i = 0; i < size; i++) {
    chain.setTip(&blocks[i]);
  }
}

static const std::vector<TestCase> cases = {
    {0, 10},
    {100, 10},
};

INSTANTIATE_TEST_SUITE_P(Chain,
                         ChainTest,
                         testing::ValuesIn(cases),
                         [](const testing::TestParamInfo<TestCase>& info) {
                           return format("%d_start%d_size%d",
                                         info.index,
                                         info.param.start,
                                         info.param.size);
                         });

TEST(ChainTest, CreateFrom0) {
  // when first block is at height 100 (no blocks behind that), and Chain is
  // created with height 0, it is expected to see that chain will contain 110
  // elements, first 100 of which are null.
  auto blocks = ChainTest::makeBlocks(100, 10);
  Chain<DummyBlock> c(0, &*blocks.rbegin());
  ASSERT_EQ(c.size(), 110);
  ASSERT_EQ(c.height(), 109);
}
