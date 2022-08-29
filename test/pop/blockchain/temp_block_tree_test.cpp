// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>
#include <veriblock/pop/blockchain/temp_block_tree.hpp>

using namespace altintegration;

template <typename T>
T getGenesisBlockHelper();

template <>
VbkBlock getGenesisBlockHelper() {
  return GetRegTestVbkBlock();
}
template <>
BtcBlock getGenesisBlockHelper() {
  return GetRegTestBtcBlock();
}

using BtcBlockTree = typename VbkBlockTree::BtcTree;

template <typename TreeType>
TreeType& getTree(AltBlockTree& tree);

template <>
VbkBlockTree& getTree<VbkBlockTree>(AltBlockTree& tree) {
  return tree.vbk();
}

template <>
BtcBlockTree& getTree<BtcBlockTree>(AltBlockTree& tree) {
  return tree.btc();
}

template <typename block_t>
std::shared_ptr<block_t> mineBlock(MockMiner& mock_miner);

template <>
std::shared_ptr<VbkBlock> mineBlock<VbkBlock>(MockMiner& mock_miner) {
  return std::make_shared<VbkBlock>(mock_miner.mineVbkBlocks(1)->getHeader());
}

template <>
std::shared_ptr<BtcBlock> mineBlock<BtcBlock>(MockMiner& mock_miner) {
  return std::make_shared<BtcBlock>(mock_miner.mineBtcBlocks(1)->getHeader());
}

template <typename block_t>
std::shared_ptr<block_t> mineBlock(const typename block_t::hash_t& hash,
                                   MockMiner& mock_miner);

template <>
std::shared_ptr<VbkBlock> mineBlock<VbkBlock>(
    const typename VbkBlock::hash_t& hash, MockMiner& mock_miner) {
  auto* tip = mock_miner.vbk().getBlockIndex(hash);
  VBK_ASSERT(tip != nullptr);
  return std::make_shared<VbkBlock>(
      mock_miner.mineVbkBlocks(1, *tip)->getHeader());
}

template <>
std::shared_ptr<BtcBlock> mineBlock<BtcBlock>(
    const typename BtcBlock::hash_t& hash, MockMiner& mock_miner) {
  auto* tip = mock_miner.btc().getBlockIndex(hash);
  return std::make_shared<BtcBlock>(
      mock_miner.mineBtcBlocks(1, *tip)->getHeader());
}

template <typename BlockTreeType>
struct TempBlockTreeTest : public ::testing::Test, public PopTestFixture {
  using block_tree_t = BlockTreeType;
  using block_t = typename BlockTreeType::block_t;

  TempBlockTreeTest() : tmp(getTree<block_tree_t>(alttree)) {}

  TempBlockTree<BlockTreeType> tmp;
};

TYPED_TEST_SUITE_P(TempBlockTreeTest);

TYPED_TEST_P(TempBlockTreeTest, scenario_1) {
  using block_t = typename TypeParam::block_t;

  auto gen_block = getGenesisBlockHelper<block_t>();

  EXPECT_NE(this->tmp.getBlockIndex(gen_block.getHash()), nullptr);
  EXPECT_NE(this->tmp.getStableTree().getBlockIndex(gen_block.getHash()),
            nullptr);
  EXPECT_EQ(this->tmp.getBlockIndex(gen_block.getHash()),
            this->tmp.getStableTree().getBlockIndex(gen_block.getHash()));

  auto block = mineBlock<block_t>(this->popminer);

  EXPECT_EQ(this->tmp.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);

  EXPECT_TRUE(this->tmp.acceptBlockHeader(block, this->state));

  EXPECT_NE(this->tmp.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);

  EXPECT_EQ(this->tmp.getBlockIndex(block->getHash())->getHash(),
            block->getHash());

  EXPECT_TRUE(areOnSameChain(*block, gen_block, this->tmp));
  EXPECT_TRUE(areOnSameChain(gen_block, *block, this->tmp));

  mineBlock<block_t>(this->popminer);
  block = mineBlock<block_t>(this->popminer);

  EXPECT_FALSE(this->tmp.acceptBlockHeader(block, this->state));

  EXPECT_EQ(this->tmp.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);
}

TYPED_TEST_P(TempBlockTreeTest, scenario_2) {
  using block_t = typename TypeParam::block_t;

  // mine 5 blocks
  std::shared_ptr<block_t> fork_point;
  for (int i = 0; i < 5; ++i) {
    fork_point = mineBlock<block_t>(this->popminer);
    EXPECT_TRUE(this->tmp.acceptBlockHeader(fork_point, this->state));
    EXPECT_NE(this->tmp.getBlockIndex(fork_point->getHash()), nullptr);
    EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(fork_point->getHash()),
              nullptr);
  }

  auto fork1 = mineBlock<block_t>(fork_point->getHash(), this->popminer);
  EXPECT_TRUE(this->tmp.acceptBlockHeader(fork1, this->state));

  auto fork2 = mineBlock<block_t>(fork_point->getHash(), this->popminer);
  EXPECT_TRUE(this->tmp.acceptBlockHeader(fork2, this->state));

  EXPECT_FALSE(areOnSameChain(*fork1, *fork2, this->tmp));
  EXPECT_FALSE(areOnSameChain(*fork2, *fork1, this->tmp));
}

// test cleanUpStaleBlocks() method
TYPED_TEST_P(TempBlockTreeTest, scenario_3) {
  using block_tree_t = TypeParam;
  using block_t = typename TypeParam::block_t;

  auto gen_block = getGenesisBlockHelper<block_t>();

  EXPECT_NE(this->tmp.getBlockIndex(gen_block.getHash()), nullptr);
  EXPECT_NE(this->tmp.getStableTree().getBlockIndex(gen_block.getHash()),
            nullptr);
  EXPECT_EQ(this->tmp.getBlockIndex(gen_block.getHash()),
            this->tmp.getStableTree().getBlockIndex(gen_block.getHash()));

  auto block = mineBlock<block_t>(this->popminer);

  EXPECT_EQ(this->tmp.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);

  EXPECT_TRUE(this->tmp.acceptBlockHeader(block, this->state));

  EXPECT_NE(this->tmp.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);

  // add block into the stable tree
  auto& stable_tree = getTree<block_tree_t>(this->alttree);

  EXPECT_TRUE(stable_tree.acceptBlockHeader(block, this->state));

  EXPECT_NE(this->tmp.getTempBlockIndex(block->getHash()), nullptr);
  EXPECT_NE(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);

  // cleanUpStaleBlocks temp tree
  this->tmp.cleanUpStaleBlocks();

  EXPECT_EQ(this->tmp.getTempBlockIndex(block->getHash()), nullptr);
  EXPECT_NE(this->tmp.getStableTree().getBlockIndex(block->getHash()), nullptr);
}

TYPED_TEST_P(TempBlockTreeTest, TempTreeDoesNotAffectRealTree) {
  using block_tree_t = TypeParam;
  using block_t = typename TypeParam::block_t;
  auto gb = getGenesisBlockHelper<block_t>();
  ValidationState state;

  auto& tmp = this->tmp;
  auto& orig = getTree<block_tree_t>(this->alttree);
  EXPECT_NE(tmp.getBlockIndex(gb.getHash()), nullptr);
  EXPECT_NE(orig.getBlockIndex(gb.getHash()), nullptr);
  EXPECT_EQ(tmp.getBlockIndex(gb.getHash()), orig.getBlockIndex(gb.getHash()));

  // accept to real tree block 1
  auto b1 = mineBlock<block_t>(this->popminer);
  ASSERT_TRUE(orig.acceptBlockHeader(*b1, state));

  // accept to tmp tree block 2
  auto b2 = mineBlock<block_t>(this->popminer);
  ASSERT_TRUE(tmp.acceptBlockHeader(b2, state));

  // check that b1 does not have b2 in pnext
  auto* ib1 = orig.getBlockIndex(b1->getHash());
  ASSERT_TRUE(ib1);

  ASSERT_TRUE(ib1->pnext.empty());
}

REGISTER_TYPED_TEST_SUITE_P(TempBlockTreeTest,
                            scenario_1,
                            scenario_2,
                            scenario_3,
                            TempTreeDoesNotAffectRealTree);

// clang-format off
typedef ::testing::Types<
    VbkBlockTree,
    BtcBlockTree
  > TypesUnderTest;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(TempBlockTreeTestSuite,
                               TempBlockTreeTest,
                               TypesUnderTest);
