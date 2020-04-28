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
  //           |\6-7-8-9      (c)
  //           |  \7-8        (d)
  //           |    \8        (e)
  //           |
  //           |\6-7-8        (f)
  //           |\6-7          (g)
  //            \6            (h)
  // best chain is (b)
  // invalidate block 5 at (b)
  // expect chain  (a) to become new best
  // expect chains (c-h) are completely invalid
  // expect chain  (b) blocks 5-10 to be invalidated

  auto& btc = popminer.btc();
  auto* fourth = btc.getBestChain().tip()->getAncestor(4);
  auto* fifth = btc.getBestChain().tip()->getAncestor(5);
  auto* Atip = popminer.mineBtcBlocks(*fourth, 5);
  auto* Btip = btc.getBestChain().tip();
  auto* Ctip = popminer.mineBtcBlocks(*fifth, 4);
  auto* Dtip = popminer.mineBtcBlocks(*Ctip->getAncestor(6), 2);
  auto* Etip = popminer.mineBtcBlocks(*Dtip->getAncestor(7), 1);
  auto* Ftip = popminer.mineBtcBlocks(*fifth, 3);  // 6-7-8
  auto* Gtip = popminer.mineBtcBlocks(*fifth, 2);  // 6-7
  auto* Htip = popminer.mineBtcBlocks(*fifth, 1);  // 6

  ASSERT_EQ(btc.getValidBlocks().size(), 11 + 5 + 4 + 2 + 1 + 3 + 2 + 1);

  // current best is B
  ASSERT_EQ(*best->tip(), *Btip);

  // invalidate block (5) on the main chain
  btc.invalidateBlockByIndex(fifth);

  // chain A is now best
  ASSERT_EQ(*best, btc.getBestChain());
  ASSERT_EQ(*best->tip(), *btc.getBestChain().tip());

  Chain<BlockIndex<BtcBlock>> Achain(0, Atip);
  Chain<BlockIndex<BtcBlock>> Bchain(5, Btip);
  Chain<BlockIndex<BtcBlock>> Cchain(6, Ctip);
  Chain<BlockIndex<BtcBlock>> Dchain(7, Dtip);
  Chain<BlockIndex<BtcBlock>> Echain(8, Etip);
  Chain<BlockIndex<BtcBlock>> Fchain(6, Ftip);
  Chain<BlockIndex<BtcBlock>> Gchain(6, Gtip);
  Chain<BlockIndex<BtcBlock>> Hchain(6, Htip);

  ASSERT_EQ(btc.getFailedBlocks().size(), 6 + 4 + 2 + 1 + 3 + 2 + 1);

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
  forEach(Gchain, getFailedBlockIndex);
  forEach(Hchain, getFailedBlockIndex);

  // there's only one chain (no known forks)
  const auto& forkChains = btc.getForkChains();
  ASSERT_TRUE(forkChains.empty());
}