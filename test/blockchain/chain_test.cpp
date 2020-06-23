// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/vbkblock.hpp>

#include "util/test_utils.hpp"

using namespace altintegration;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

struct DummyAddon {};

struct MyDummyBlock {
  using addon_t = DummyAddon;
  using hash_t = int;
  using prev_hash_t = int;
  using height_t = int;
  using protecting_block_t = std::false_type;
};

struct TestCase {
  int start;
  int size;
};

struct ChainTest : public ::testing::TestWithParam<TestCase> {
  Chain<BlockIndex<MyDummyBlock>> chain{};

  static std::vector<BlockIndex<MyDummyBlock>> makeBlocks(int startHeight,
                                                          int size) {
    std::vector<BlockIndex<MyDummyBlock>> blocks;
    for (int i = 0; i < size; i++) {
      BlockIndex<MyDummyBlock> index{};
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
  chain = Chain<BlockIndex<MyDummyBlock>>(start, &*blocks.rbegin());
  EXPECT_EQ(chain.chainHeight(), start + size - 1);
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

INSTANTIATE_TEST_SUITE_P(Chain, ChainTest, testing::ValuesIn(cases));

TEST(ChainTest, ChainStartHeightAboveTip) {
  auto blocks = ChainTest::makeBlocks(0, 10);
  Chain<BlockIndex<MyDummyBlock>> chain(100, &*blocks.rbegin());
  ASSERT_TRUE(chain.empty());
}

TEST(ChainTest, CreateFrom0) {
  // when first block is at height 100 (no blocks behind that), and Chain is
  // created with height 0, it is expected to see that chain will contain 110
  // elements, first 100 of which are null.
  auto blocks = ChainTest::makeBlocks(100, 10);
  Chain<BlockIndex<MyDummyBlock>> c(0, &*blocks.rbegin());
  ASSERT_EQ(c.blocksCount(), 110);
  ASSERT_EQ(c.chainHeight(), 109);
}

template <typename Block, typename Endorsement>
Endorsement generateEndorsement(const Block& endorsedBlock,
                                const Block& containingBlock) {
  Endorsement endorsement;
  endorsement.id = generateRandomBytesVector(32);
  endorsement.endorsedHash = endorsedBlock.getHash();
  endorsement.containingHash = containingBlock.getHash();
  endorsement.blockOfProof = generateRandomBytesVector(10);
  return endorsement;
}

template <typename Block>
BlockIndex<Block> generateNextBlock(BlockIndex<Block>* prev);

template <>
BlockIndex<AltBlock> generateNextBlock(BlockIndex<AltBlock>* prev) {
  AltBlock block;
  block.hash = generateRandomBytesVector(32);
  if (prev != nullptr) {
    block.height = prev->height + 1;
    block.previousBlock = prev->getHash();
    block.timestamp = prev->header->timestamp + 1;
  } else {
    block.height = 0;
    block.timestamp = 0;
  }

  BlockIndex<AltBlock> index;
  index.header = std::make_shared<AltBlock>(block);
  index.height = block.height;
  index.pprev = prev;
  return index;
}

template <>
BlockIndex<VbkBlock> generateNextBlock(BlockIndex<VbkBlock>* prev) {
  VbkBlock block;
  if (prev != nullptr) {
    block.height = prev->height + 1;
    block.previousBlock = prev->getHash().trimLE<uint96::size()>();
    block.timestamp = prev->header->timestamp + 1;
  } else {
    block.height = 0;
    block.timestamp = 0;
    block.nonce = 0;
    block.version = 0;
  }

  BlockIndex<VbkBlock> index;
  index.header = std::make_shared<VbkBlock>(block);
  index.height = block.height;
  index.pprev = prev;
  return index;
}

template <typename Block>
struct ChainTestFixture : public ::testing::Test {
  using block_t = Block;
  using index_t = BlockIndex<Block>;
  using hash_t = typename block_t::hash_t;
  using height_t = typename block_t::height_t;
  using endorsement_t = typename index_t::endorsement_t;
};

TYPED_TEST_SUITE_P(ChainTestFixture);

TYPED_TEST_P(ChainTestFixture, findEndorsement) {
  using block_t = typename findEndorsement::block_t;
  using endorsement_t = typename findEndorsement::endorsement_t;

  BlockIndex<block_t> bootstrapBlock = generateNextBlock<block_t>(nullptr);

  Chain<BlockIndex<block_t>> chain(bootstrapBlock.height, &bootstrapBlock);

  std::vector<std::shared_ptr<BlockIndex<block_t>>> indexes{
      std::make_shared<BlockIndex<block_t>>(bootstrapBlock)};

  for (int i = 0; i < 10; ++i) {
    auto block =
        std::make_shared<BlockIndex<block_t>>(generateNextBlock(chain.tip()));
    indexes.push_back(block);
    chain.setTip(block.get());
  }

  BlockIndex<block_t> newIndex = generateNextBlock(chain.tip());

  endorsement_t endorsement1 = generateEndorsement<block_t, endorsement_t>(
      *chain.tip()->header, *newIndex.header);
  endorsement_t endorsement2 = generateEndorsement<block_t, endorsement_t>(
      *chain.tip()->pprev->header, *newIndex.header);

  newIndex.containingEndorsements.insert(std::make_pair(
      endorsement1.id, std::make_shared<endorsement_t>(endorsement1)));
  newIndex.containingEndorsements.insert(std::make_pair(
      endorsement2.id, std::make_shared<endorsement_t>(endorsement2)));

  chain.setTip(&newIndex);

  BlockIndex<block_t> newIndex2 = generateNextBlock(chain.tip());

  endorsement_t endorsement3 = generateEndorsement<block_t, endorsement_t>(
      *chain.tip()->header, *newIndex2.header);
  endorsement_t endorsement4 = generateEndorsement<block_t, endorsement_t>(
      *chain.tip()->pprev->header, *newIndex2.header);

  newIndex2.containingEndorsements.insert(std::make_pair(
      endorsement3.id, std::make_shared<endorsement_t>(endorsement3)));

  chain.setTip(&newIndex2);

  EXPECT_EQ(*findBlockContainingEndorsement(chain, endorsement1, 100)
                 ->containingEndorsements.find(endorsement1.id)
                 ->second,
            endorsement1);
  EXPECT_EQ(*findBlockContainingEndorsement(chain, endorsement2, 100)
                 ->containingEndorsements.find(endorsement2.id)
                 ->second,
            endorsement2);
  EXPECT_EQ(*findBlockContainingEndorsement(chain, endorsement3, 100)
                 ->containingEndorsements.find(endorsement3.id)
                 ->second,
            endorsement3);
  EXPECT_EQ(findBlockContainingEndorsement(chain, endorsement4, 100), nullptr);
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(ChainTestFixture, findEndorsement);

typedef ::testing::Types<AltBlock, VbkBlock> TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(ChainTestSuite,
                               ChainTestFixture,
                               TypesUnderTest);
