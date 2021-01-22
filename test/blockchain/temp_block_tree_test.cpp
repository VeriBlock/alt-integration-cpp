// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

#include "veriblock/blockchain/temp_block_tree.hpp"

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
std::shared_ptr<block_t> mineBlock(MockMiner2& mock_miner);

template <>
std::shared_ptr<VbkBlock> mineBlock<VbkBlock>(MockMiner2& mock_miner) {
  return std::make_shared<VbkBlock>(mock_miner.mineVbkBlocks(1)->getHeader());
}

template <>
std::shared_ptr<BtcBlock> mineBlock<BtcBlock>(MockMiner2& mock_miner) {
  return std::make_shared<BtcBlock>(mock_miner.mineBtcBlocks(1)->getHeader());
}

template <typename block_t>
std::shared_ptr<block_t> mineBlock(const typename block_t::hash_t& hash,
                                   MockMiner2& mock_miner);

template <>
std::shared_ptr<VbkBlock> mineBlock<VbkBlock>(
    const typename VbkBlock::hash_t& hash, MockMiner2& mock_miner) {
  auto* tip = mock_miner.vbk().getBlockIndex(hash);
  VBK_ASSERT(tip != nullptr);
  return std::make_shared<VbkBlock>(
      mock_miner.mineVbkBlocks(1, *tip)->getHeader());
}

template <>
std::shared_ptr<BtcBlock> mineBlock<BtcBlock>(
    const typename BtcBlock::hash_t& hash, MockMiner2& mock_miner) {
  auto* tip = mock_miner.btc().getBlockIndex(hash);
  return std::make_shared<BtcBlock>(
      mock_miner.mineBtcBlocks(1, *tip)->getHeader());
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

  auto gen_block = getGenesisBlockHelper<block_t>();

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

TYPED_TEST_P(TempBlockTreeTest, scenario_2) {
  using block_t = typename TypeParam::block_t;

  // mine 5 blocks
  std::shared_ptr<block_t> fork_point;
  for (int i = 0; i < 5; ++i) {
    fork_point = mineBlock<block_t>(*this->popminer);
    EXPECT_TRUE(this->temp_block_tree.acceptBlock(fork_point, this->state));
    EXPECT_NE(this->temp_block_tree.getBlockIndex(fork_point->getHash()),
              nullptr);
    EXPECT_EQ(this->temp_block_tree.getStableTree().getBlockIndex(
                  fork_point->getHash()),
              nullptr);
  }

  auto fork1 = mineBlock<block_t>(fork_point->getHash(), *this->popminer);
  EXPECT_TRUE(this->temp_block_tree.acceptBlock(fork1, this->state));

  auto fork2 = mineBlock<block_t>(fork_point->getHash(), *this->popminer);
  EXPECT_TRUE(this->temp_block_tree.acceptBlock(fork2, this->state));

  EXPECT_FALSE(areOnSameChain(*fork1, *fork2, this->temp_block_tree));
  EXPECT_FALSE(areOnSameChain(*fork2, *fork1, this->temp_block_tree));
}

// test cleanUp() method
TYPED_TEST_P(TempBlockTreeTest, scenario_3) {
  using block_tree_t = TypeParam;
  using block_t = typename TypeParam::block_t;

  auto gen_block = getGenesisBlockHelper<block_t>();

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

  // add block into the stable tree
  auto& stable_tree = getTree<block_tree_t>(this->alttree);

  EXPECT_TRUE(stable_tree.acceptBlock(block, this->state));

  EXPECT_NE(this->temp_block_tree.getTempBlockIndex(block->getHash()), nullptr);
  EXPECT_NE(
      this->temp_block_tree.getStableTree().getBlockIndex(block->getHash()),
      nullptr);

  // cleanUp temp tree
  this->temp_block_tree.cleanUp();

  EXPECT_EQ(this->temp_block_tree.getTempBlockIndex(block->getHash()), nullptr);
  EXPECT_NE(
      this->temp_block_tree.getStableTree().getBlockIndex(block->getHash()),
      nullptr);
}

REGISTER_TYPED_TEST_SUITE_P(TempBlockTreeTest,
                            scenario_1,
                            scenario_2,
                            scenario_3);

// clang-format off
typedef ::testing::Types<
    VbkBlockTree,
    BtcBlockTree
  > TypesUnderTest;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(TempBlockTreeTestSuite,
                               TempBlockTreeTest,
                               TypesUnderTest);