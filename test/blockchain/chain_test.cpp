#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/fmt.hpp>

#include "util/test_utils.hpp"

using namespace altintegration;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

struct DummyBlock {
  using hash_t = int;
  using height_t = int;
  using payloads_t = DummyPayloads;
  using endorsement_t = DummyEndorsement;
};

struct TestCase {
  int start;
  int size;
};

struct ChainTest : public ::testing::TestWithParam<TestCase> {
  Chain<BlockIndex<DummyBlock>> chain{};

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
  chain = Chain<BlockIndex<DummyBlock>>(start, &*blocks.rbegin());
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
  Chain<BlockIndex<DummyBlock>> c(0, &*blocks.rbegin());
  ASSERT_EQ(c.blocksCount(), 110);
  ASSERT_EQ(c.chainHeight(), 109);
}

template <typename EndorsementType>
EndorsementType generateEndorsement();

template <typename Block>
BlockIndex<Block> generateNextBlock(BlockIndex<Block>* prev);

template <>
BlockIndex<AltBlock> generateNextBlock(BlockIndex<AltBlock>* prev) {
  AltBlock block;
  block.hash = generateRandomBytesVector(32);
  block.height = prev->height + 1;
  block.previousBlock = prev->getHash();
  block.timestamp = prev->header.timestamp + 1;

  BlockIndex<AltBlock> index;
  index.header = block;
  index.height = block.height;
  index.pprev = prev;
  return index;
}

template <typename Block>
struct ChainTestFixture : public ::testing::Test {
  using block_t = Block;
  using hash_t = typename block_t::hash_t;
  using height_t = typename block_t::height_t;
  using endorsement_t = typename block_t::endorsement_t;
  using eid_t = typename endorsement_t::id_t;
};

TYPED_TEST_SUITE_P(ChainTestFixture);

TYPED_TEST_P(ChainTestFixture, findEndorsement) {}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(ChainTestFixture, findEndorsement);

typedef ::testing::Types<> TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(ChainTestSuite,
                               ChainTestFixture,
                               TypesUnderTest);

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

TEST(ChainTest, findEndorsement) {
  AltBlock bootstrapBlock = AltChainParamsTest().getBootstrapBlock();
  BlockIndex<AltBlock> bootstrapBlockIndex;
  bootstrapBlock.height = bootstrapBlock.height;
  bootstrapBlockIndex.header = bootstrapBlock;
  bootstrapBlockIndex.pprev = nullptr;

  std::vector<std::shared_ptr<BlockIndex<AltBlock>>> indexes{
      std::make_shared<BlockIndex<AltBlock>>(bootstrapBlockIndex)};

  Chain<BlockIndex<AltBlock>> chain(bootstrapBlockIndex.height,
                                    &bootstrapBlockIndex);

  for (int i = 0; i < 10; ++i) {
    std::shared_ptr<BlockIndex<AltBlock>> block =
        std::make_shared<BlockIndex<AltBlock>>(generateNextBlock(chain.tip()));
    indexes.push_back(block);
    chain.setTip(block.get());
  }

  VbkEndorsement endorsement1;
  endorsement1.id = generateRandomBytesVector(32);
  endorsement1.blockOfProof = generateRandomBytesVector(10);
  endorsement1.containingHash = generateRandomBytesVector(10);

  VbkEndorsement endorsement2;
  endorsement2.id = generateRandomBytesVector(32);
  endorsement2.blockOfProof = generateRandomBytesVector(10);
  endorsement2.containingHash = generateRandomBytesVector(10);

  VbkEndorsement endorsement3;
  endorsement3.id = generateRandomBytesVector(32);
  endorsement3.blockOfProof = generateRandomBytesVector(10);
  endorsement3.containingHash = generateRandomBytesVector(10);

  VbkEndorsement endorsement4;
  endorsement4.id = generateRandomBytesVector(32);
  endorsement4.blockOfProof = generateRandomBytesVector(10);
  endorsement4.containingHash = generateRandomBytesVector(10);

  BlockIndex<AltBlock> newIndex = generateNextBlock(chain.tip());

  newIndex.containingEndorsements[endorsement1.id] =
      std::make_shared<VbkEndorsement>(endorsement1);
  newIndex.containingEndorsements[endorsement2.id] =
      std::make_shared<VbkEndorsement>(endorsement2);

  chain.setTip(&newIndex);

  BlockIndex<AltBlock> newIndex2 = generateNextBlock(chain.tip());

  newIndex2.containingEndorsements[endorsement3.id] =
      std::make_shared<VbkEndorsement>(endorsement3);

  chain.setTip(&newIndex2);
  EXPECT_EQ(*chain.findEndorsement(endorsement1.id, 100), endorsement1);
  EXPECT_EQ(*chain.findEndorsement(endorsement2.id, 100), endorsement2);
  EXPECT_EQ(*chain.findEndorsement(endorsement3.id, 100), endorsement3);
  EXPECT_EQ(chain.findEndorsement(endorsement4.id, 100), nullptr);
}
