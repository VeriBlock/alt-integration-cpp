// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

#include "veriblock/blockchain/temp_block_tree.hpp"

using namespace altintegration;

using BtcBlockTree = typename VbkBlockTree::BtcTree;

template <typename TreeType>
const TreeType& getTree(const AltTree& tree);

template <>
const VbkBlockTree& getTree<VbkBlockTree>(const AltTree& tree) {
  return tree.vbk();
}

template <>
const BtcBlockTree& getTree<BtcBlockTree>(const AltTree& tree) {
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
std::shared_ptr<block_t> mineBlock(const BlockIndex<block_t>& tip,
                                   MockMiner& mock_miner);

template <>
std::shared_ptr<VbkBlock> mineBlock<VbkBlock>(const BlockIndex<VbkBlock>& tip,
                                              MockMiner& mock_miner) {
  return std::make_shared<VbkBlock>(
      mock_miner.mineVbkBlocks(tip, 1)->getHeader());
}

template <>
std::shared_ptr<BtcBlock> mineBlock<BtcBlock>(const BlockIndex<BtcBlock>& tip,
                                              MockMiner& mock_miner) {
  return std::make_shared<BtcBlock>(
      mock_miner.mineBtcBlocks(tip, 1)->getHeader());
}

template <typename BlockTreeType>
struct TempBlockTreeTest : public ::testing::Test, public PopTestFixture {
  using block_tree_t = BlockTreeType;
  using block_t = typename BlockTreeType::block_t;

  TempBlockTreeTest() : temp_block_tree(getTree<block_tree_t>(alttree)) {}

  TempBlockTree<BlockTreeType> temp_block_tree;
};

TYPED_TEST_SUITE_P(TempBlockTreeTest);

TYPED_TEST_P(TempBlockTreeTest, scenario_1) {
  using block_t = typename TypeParam::block_t;

  auto gen_block =
      this->temp_block_tree.getStableTree().getParams().getGenesisBlock();

  EXPECT_NE(this->temp_block_tree.getBlockIndex(gen_block.getHash()), nullptr);
  EXPECT_NE(
      this->temp_block_tree.getStableTree().getBlockIndex(gen_block.getHash()),
      nullptr);
  EXPECT_EQ(
      this->temp_block_tree.getBlockIndex(gen_block.getHash()),
      this->temp_block_tree.getStableTree().getBlockIndex(gen_block.getHash()));

  auto block = mineBlock<block_t>(*this->popminer);

  EXPECT_EQ(this->temp_block_tree.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(
      this->temp_block_tree.getStableTree().getBlockIndex(block->getHash()),
      nullptr);

  EXPECT_TRUE(this->temp_block_tree.acceptBlock(block, this->state));

  EXPECT_NE(this->temp_block_tree.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(
      this->temp_block_tree.getStableTree().getBlockIndex(block->getHash()),
      nullptr);

  EXPECT_EQ(this->temp_block_tree.getBlockIndex(block->getHash())->getHash(),
            block->getHash());

  EXPECT_TRUE(areOnSameChain(*block, gen_block, this->temp_block_tree));
  EXPECT_TRUE(areOnSameChain(gen_block, *block, this->temp_block_tree));

  mineBlock<block_t>(*this->popminer);
  block = mineBlock<block_t>(*this->popminer);

  EXPECT_FALSE(this->temp_block_tree.acceptBlock(block, this->state));

  EXPECT_EQ(this->temp_block_tree.getBlockIndex(block->getHash()), nullptr);
  EXPECT_EQ(
      this->temp_block_tree.getStableTree().getBlockIndex(block->getHash()),
      nullptr);
}

REGISTER_TYPED_TEST_SUITE_P(TempBlockTreeTest, scenario_1);

// clang-format off
typedef ::testing::Types<
    VbkBlockTree,
    BtcBlockTree
  > TypesUnderTest;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(TempBlockTreeTestSuite,
                               TempBlockTreeTest,
                               TypesUnderTest);