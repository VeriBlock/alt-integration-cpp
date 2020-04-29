// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct BlockchainFixture : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* tip;
  const Chain<BlockIndex<BtcBlock>>* best;

  BlockchainFixture() {
    tip = popminer.mineBtcBlocks(10);
    best = &popminer.btc().getBestChain();
    EXPECT_EQ(*best->tip(), *tip);
    EXPECT_EQ(tip->status, BLOCK_VALID_TREE);
    EXPECT_TRUE(tip->isValid());
  }

  template <typename T, typename F>
  void forEach(const T& chain, F&& check) {
    for (const auto& c : chain) {
      check(c->getHash());
    }
  };
};

TEST_F(BlockchainFixture, InvalidateTip) {
  popminer.btc().invalidateTip();

  // block is not removed
  ASSERT_NE(tip, nullptr);
  // block is not valid
  ASSERT_FALSE(tip->isValid());
  // has tree validity, but we marked it as invalid
  ASSERT_EQ(tip->status, BLOCK_VALID_TREE | BLOCK_FAILED_BLOCK);
  ASSERT_EQ(popminer.btc().getFailedBlocks().size(), 1);
  ASSERT_EQ(*popminer.btc().getFailedBlocks().begin()->second, *tip);
}

TEST_F(BlockchainFixture, InvalidateBlockInTheMiddleOfChain) {
  // invalidate block at height 5
  auto* toBeInvalidated = tip->getAncestor(5);
  ASSERT_TRUE(toBeInvalidated);

  // backup chain
  Chain<BlockIndex<BtcBlock>> chain(0, tip);

  // invalidate block #5
  auto& btc = popminer.btc();
  btc.invalidateBlockByIndex(toBeInvalidated);

  // valid block map has 5 blocks (0,1,2,3,4)
  ASSERT_EQ(btc.getValidBlocks().size(), 5);

  // invalid block map has 6 blocks (5,6,7,8,9,10)
  ASSERT_EQ(btc.getFailedBlocks().size(), 6);

  // block #5 is marked as BLOCK_FAILED_BLOCK
  ASSERT_TRUE(chain[5]->status & BLOCK_FAILED_BLOCK);

  // all next blocks are marked as BLOCK_FAILED_CHILD
  BlockIndex<BtcBlock>* current = chain.next(toBeInvalidated);
  do {
    ASSERT_TRUE(current->status & BLOCK_FAILED_CHILD);
    current = chain.next(current);
  } while (current != nullptr);
}

TEST_F(BlockchainFixture, InvalidBlockAsBaseOfMultipleForks) {
  //          /5-6-7-8-9      (a)
  // 0-1-2-3-4-5-6-7-8-9-10   (b)
  //             |\7-8-9      (c)
  //             |  \8        (d)
  //             |\7-8        (e)
  //              \7          (f)
  // best chain is (b)
  // invalidate block 6 at (b)
  // expect chain  (a) to become new best
  // expect chains (c-f) are completely invalid
  // expect chain  (b) blocks 6-10 to be invalidated
  // expect block 5 from (b) to be added to forkChains as new candidate fork

  auto& btc = popminer.btc();
  auto* fourth = btc.getBestChain().tip()->getAncestor(4);
  auto* sixth = btc.getBestChain().tip()->getAncestor(6);
  auto* Atip = popminer.mineBtcBlocks(*fourth, 5);
  auto* Btip = btc.getBestChain().tip();
  auto* Ctip = popminer.mineBtcBlocks(*sixth, 3);
  auto* Dtip = popminer.mineBtcBlocks(*Ctip->getAncestor(7), 1);
  auto* Etip = popminer.mineBtcBlocks(*sixth, 2);  // 7-8
  auto* Ftip = popminer.mineBtcBlocks(*sixth, 1);  // 9

  ASSERT_EQ(btc.getValidBlocks().size(), 11 + 5 + 3 + 1 + 2 + 1);

  // current best is B
  ASSERT_EQ(*best->tip(), *Btip);

  // invalidate block (5) on the main chain
  btc.invalidateBlockByIndex(sixth);

  // chain A is now best
  ASSERT_EQ(*best, btc.getBestChain());
  ASSERT_EQ(*best->tip(), *btc.getBestChain().tip());

  Chain<BlockIndex<BtcBlock>> Achain(0, Atip);
  Chain<BlockIndex<BtcBlock>> Bchain(6, Btip);
  Chain<BlockIndex<BtcBlock>> Cchain(7, Ctip);
  Chain<BlockIndex<BtcBlock>> Dchain(8, Dtip);
  Chain<BlockIndex<BtcBlock>> Echain(7, Etip);
  Chain<BlockIndex<BtcBlock>> Fchain(7, Ftip);

  ASSERT_EQ(btc.getFailedBlocks().size(), 5 + 3 + 1 + 2 + 1);

  auto getValidBlockIndex = [&](auto&& hash) {
    auto index = btc.getBlockIndex(hash);
    EXPECT_TRUE(index);
    EXPECT_TRUE(index->isValid());
    return index;
  };

  auto getFailedBlockIndex = [&](auto&& hash) {
    auto index = btc.getBlockIndexFailed(hash);
    EXPECT_TRUE(index);
    EXPECT_FALSE(index->isValid());
    return index;
  };

  // all blocks from A exist in VALID_BLOCKS
  forEach(Achain, getValidBlockIndex);
  // all blocks from these chains exist in FAILED_BLOCKS
  forEach(Bchain, getFailedBlockIndex);
  forEach(Cchain, getFailedBlockIndex);
  forEach(Dchain, getFailedBlockIndex);
  forEach(Echain, getFailedBlockIndex);
  forEach(Fchain, getFailedBlockIndex);

  // there's only one chain (no known forks)
  const auto& forkChains = btc.getForkChains();
  ASSERT_EQ(forkChains.size(), 1);
  // block 5 at (b) should become new fork chain
  ASSERT_EQ(*forkChains.begin()->second.tip(), *Btip->getAncestor(5));
}