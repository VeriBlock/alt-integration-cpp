// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/chain.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>

#include "pop/util/test_utils.hpp"

using namespace altintegration;

struct DummyAddon {
  std::string toPrettyString() const { return {}; }
  void setNull(){};
};

struct MyDummyBlock {
  using addon_t = DummyAddon;
  using hash_t = std::vector<uint8_t>;
  using prev_hash_t = int;
  using height_t = int;
  using protecting_block_t = std::false_type;
  static std::string name() { return {}; };
  hash_t h;
  const hash_t& getHash() const { return h; };

  std::string toPrettyString() const { return "MyDummyBlock"; }
};

struct BlocksOwner {
  std::vector<std::shared_ptr<BlockIndex<MyDummyBlock>>> blocks;

  ~BlocksOwner() {
    // destroy chain in reverse order (from tip to root)
    for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
      it->reset();
    }
  }
};

struct TestCase {
  int start;
  int size;
};

struct ChainTest : public ::testing::TestWithParam<TestCase> {
  Chain<BlockIndex<MyDummyBlock>> chain{};

  static BlocksOwner makeBlocks(int startHeight, int size) {
    std::vector<std::shared_ptr<BlockIndex<MyDummyBlock>>> blocks;

    // bootstrap
    if (size > 0) {
      blocks.push_back(std::make_shared<BlockIndex<MyDummyBlock>>(startHeight));
    }

    for (int i = 1; i < size; i++) {
      blocks.push_back(
          std::make_shared<BlockIndex<MyDummyBlock>>(blocks[i - 1].get()));
    }

    return {blocks};
  }
};

TEST_P(ChainTest, Full) {
  auto [start, size] = GetParam();
  auto owner = makeBlocks(start, size);
  auto& blocks = owner.blocks;
  chain = Chain<BlockIndex<MyDummyBlock>>(start, blocks.back().get());
  EXPECT_EQ(chain.chainHeight(), start + size - 1);
  EXPECT_EQ(chain.tip(), blocks.back().get());

  // check 'contains' method
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(chain[i + start], blocks[i].get());
    EXPECT_TRUE(chain.contains(blocks[i].get()));
  }
  EXPECT_EQ(chain[start - 1], nullptr);

  // check out of range blocks
  EXPECT_EQ(chain[-1], nullptr);
  EXPECT_EQ(chain[start + size + 1], nullptr);

  // check 'next' method
  for (int i = 0; i < size - 2; i++) {
    EXPECT_EQ(chain.next(blocks[i].get()), (blocks[i + 1]).get());
  }
  EXPECT_EQ(chain.next(blocks[blocks.size() - 1].get()), nullptr);

  // check 'setTip' method
  for (int i = 0; i < size; i++) {
    chain.setTip(blocks[i].get());
  }
}

static const std::vector<TestCase> cases = {
    {0, 10},
    {100, 10},
};

INSTANTIATE_TEST_SUITE_P(Chain, ChainTest, testing::ValuesIn(cases));

TEST(ChainTest, ChainStartHeightAboveTip) {
  auto owner = ChainTest::makeBlocks(0, 10);
  auto& blocks = owner.blocks;
  Chain<BlockIndex<MyDummyBlock>> chain(100, blocks.back().get());
  ASSERT_TRUE(chain.empty());
}

TEST(ChainTest, CreateFrom0) {
  // when first block is at height 100 (no blocks behind that), and Chain is
  // created with height 0, it is expected to see that chain will contain 110
  // elements, first 100 of which are null.
  auto owner = ChainTest::makeBlocks(100, 10);
  auto& blocks = owner.blocks;

  Chain<BlockIndex<MyDummyBlock>> c(0, blocks.back().get());
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
std::shared_ptr<BlockIndex<Block>> generateNextBlock(BlockIndex<Block>* prev);

template <>
std::shared_ptr<BlockIndex<AltBlock>> generateNextBlock(
    BlockIndex<AltBlock>* prev) {
  AltBlock block;
  block.hash = generateRandomBytesVector(32);
  if (prev != nullptr) {
    block.height = prev->getHeight() + 1;
    block.previousBlock = prev->getHash();
    block.timestamp = prev->getHeader().getBlockTime() + 1;
  } else {
    block.height = 0;
    block.timestamp = 0;
  }
  auto index = prev == nullptr
                   ? std::make_shared<BlockIndex<AltBlock>>(block.height)
                   : std::make_shared<BlockIndex<AltBlock>>(prev);
  index->setHeader(block);
  VBK_ASSERT(index->getHeight() == block.height);
  return index;
}

template <>
std::shared_ptr<BlockIndex<VbkBlock>> generateNextBlock(
    BlockIndex<VbkBlock>* prev) {
  VbkBlock block;
  if (prev != nullptr) {
    block.setHeight(prev->getHeight() + 1);
    block.setPreviousBlock(prev->getHash().trimLE<uint96::size()>());
    block.setTimestamp(prev->getHeader().getTimestamp() + 1);
  } else {
    block.setHeight(0);
    block.setTimestamp(0);
    block.setNonce(0);
    block.setVersion(0);
  }
  auto index = prev == nullptr
                   ? std::make_shared<BlockIndex<VbkBlock>>(block.getHeight())
                   : std::make_shared<BlockIndex<VbkBlock>>(prev);
  index->setHeader(block);
  VBK_ASSERT(index->getHeight() == block.getHeight());
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

  std::vector<std::shared_ptr<BlockIndex<block_t>>> indexes{
      generateNextBlock<block_t>(nullptr)};

  auto& bootstrapBlock = indexes.at(0);
  Chain<BlockIndex<block_t>> chain(bootstrapBlock->getHeight(),
                                   bootstrapBlock.get());

  for (int i = 0; i < 10; ++i) {
    auto index = generateNextBlock(chain.tip());
    indexes.push_back(index);
    chain.setTip(index.get());
  }

  auto newIndex = generateNextBlock(chain.tip());

  endorsement_t endorsement1 = generateEndorsement<block_t, endorsement_t>(
      chain.tip()->getHeader(), newIndex->getHeader());
  endorsement_t endorsement2 = generateEndorsement<block_t, endorsement_t>(
      chain.tip()->pprev->getHeader(), newIndex->getHeader());

  newIndex->insertContainingEndorsement(
      std::make_shared<endorsement_t>(endorsement1));
  newIndex->insertContainingEndorsement(
      std::make_shared<endorsement_t>(endorsement2));

  chain.setTip(newIndex.get());

  auto newIndex2 = generateNextBlock(chain.tip());

  endorsement_t endorsement3 = generateEndorsement<block_t, endorsement_t>(
      chain.tip()->getHeader(), newIndex2->getHeader());
  endorsement_t endorsement4 = generateEndorsement<block_t, endorsement_t>(
      chain.tip()->pprev->getHeader(), newIndex2->getHeader());

  newIndex2->insertContainingEndorsement(
      std::make_shared<endorsement_t>(endorsement3));

  chain.setTip(newIndex2.get());

  auto* blockContaining1 =
      findBlockContainingEndorsement(chain, endorsement1, 100);
  EXPECT_EQ(*blockContaining1->getContainingEndorsements()
                 .find(endorsement1.id)
                 ->second,
            endorsement1);
  auto* blockContaining2 =
      findBlockContainingEndorsement(chain, endorsement2, 100);
  EXPECT_EQ(*blockContaining2->getContainingEndorsements()
                 .find(endorsement2.id)
                 ->second,
            endorsement2);
  auto* blockContaining3 =
      findBlockContainingEndorsement(chain, endorsement3, 100);
  EXPECT_EQ(*blockContaining3->getContainingEndorsements()
                 .find(endorsement3.id)
                 ->second,
            endorsement3);
  EXPECT_EQ(findBlockContainingEndorsement(chain, endorsement4, 100), nullptr);

  // deallocate blocks in reverse order (tip->root)
  newIndex2.reset();
  newIndex.reset();
  for (auto it = indexes.rbegin(); it != indexes.rend(); ++it) {
    it->reset();
  }
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(ChainTestFixture, findEndorsement);

typedef ::testing::Types<AltBlock, VbkBlock> TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(ChainTestSuite,
                               ChainTestFixture,
                               TypesUnderTest);
