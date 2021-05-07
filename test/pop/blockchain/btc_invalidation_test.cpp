// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

struct BtcInvalidationTest : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* tip;
  const Chain<BlockIndex<BtcBlock>>* best;

  BtcInvalidationTest() {
    tip = popminer->mineBtcBlocks(10);
    best = &popminer->btc().getBestChain();
    EXPECT_TRUE(cmp(*best->tip(), *tip));
    EXPECT_TRUE(tip->isValid(BLOCK_VALID_TREE));
  }

  template <typename T, typename F>
  void forEach(const T& chain, F&& check) {
    for (const auto& c : chain) {
      check(c->getHash());
    }
  };
};

TEST_F(BtcInvalidationTest, InvalidateTip) {
  popminer->btc().invalidateSubtree(
      popminer->btc().getBestChain().tip()->getHash(), BLOCK_FAILED_BLOCK);

  // block is not removed
  ASSERT_NE(tip, nullptr);
  // block is not valid
  ASSERT_FALSE(tip->isValid());
  // has tree validity, but we marked it as invalid
  EXPECT_TRUE(tip->isValidUpTo(BLOCK_VALID_TREE));
  EXPECT_TRUE(tip->hasFlags(BLOCK_FAILED_BLOCK));
}

TEST_F(BtcInvalidationTest, InvalidateBlockInTheMiddleOfChain) {
  // invalidate block at height 5
  auto* toBeInvalidated = tip->getAncestor(5);
  ASSERT_TRUE(toBeInvalidated);

  // backup chain
  Chain<BlockIndex<BtcBlock>> chain(0, tip);

  // invalidate block #5
  auto& btc = popminer->btc();
  btc.invalidateSubtree(*toBeInvalidated, BLOCK_FAILED_BLOCK);

  size_t totalValid = 0;
  size_t totalInvalid = 0;

  forEachNextNodePreorder<BtcBlock>(*tip->getAncestor(0),
                                    [&](BlockIndex<BtcBlock>& block) -> bool {
                                      if (block.isValid()) {
                                        totalValid++;
                                      } else {
                                        totalInvalid++;
                                      }

                                      return true;
                                    });

  // valid block map has 4 blocks (1,2,3,4), excliding genesis
  ASSERT_EQ(totalValid, 4);

  // invalid block map has 6 blocks (5,6,7,8,9,10)
  ASSERT_EQ(totalInvalid, 6);

  // block #5 is marked as BLOCK_FAILED_BLOCK
  ASSERT_TRUE(chain[5]->getStatus() & BLOCK_FAILED_BLOCK);

  // all next blocks are marked as BLOCK_FAILED_CHILD
  BlockIndex<BtcBlock>* current = chain.next(toBeInvalidated);
  do {
    ASSERT_TRUE(current->getStatus() & BLOCK_FAILED_CHILD);
    current = chain.next(current);
  } while (current != nullptr);

  // revalidate block 5
  popminer->btc().revalidateSubtree(toBeInvalidated->getHash(),
                                    BLOCK_FAILED_BLOCK);

  // all next blocks are valid
  current = chain.next(toBeInvalidated);
  do {
    ASSERT_TRUE(current->isValid());
    current = chain.next(current);
  } while (current != nullptr);
}

TEST_F(BtcInvalidationTest, InvalidBlockAsBaseOfMultipleForks) {
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

  auto& btc = popminer->btc();
  ASSERT_EQ(btc.getBlocks().size(), 11);
  auto* fourth = btc.getBestChain().tip()->getAncestor(4);
  auto* sixth = btc.getBestChain().tip()->getAncestor(6);
  auto* Atip = popminer->mineBtcBlocks(5, *fourth);
  ASSERT_EQ(btc.getBlocks().size(), 11 + 5);
  auto* Btip = btc.getBestChain().tip();
  auto* B5 = Btip->getAncestor(5);
  auto* Ctip = popminer->mineBtcBlocks(3, *sixth);
  ASSERT_EQ(btc.getBlocks().size(), 11 + 5 + 3);
  auto* Dtip = popminer->mineBtcBlocks(1, *Ctip->getAncestor(7));
  ASSERT_EQ(btc.getBlocks().size(), 11 + 5 + 3 + 1);
  auto* Etip = popminer->mineBtcBlocks(2, *sixth);  // 7-8
  ASSERT_EQ(btc.getBlocks().size(), 11 + 5 + 3 + 1 + 2);
  auto* Ftip = popminer->mineBtcBlocks(1, *sixth);  // 9

  ASSERT_EQ(btc.getBlocks().size(), 11 + 5 + 3 + 1 + 2 + 1);
  ASSERT_EQ(btc.getTips().size(), 6);

  // current best is B
  ASSERT_TRUE(cmp(*best->tip(), *Btip));

  // invalidate block (5) on the main chain
  btc.invalidateSubtree(*sixth, BLOCK_FAILED_BLOCK);

  // chain A is now best
  ASSERT_TRUE(cmp(*best, btc.getBestChain()));
  ASSERT_TRUE(cmp(*best->tip(), *btc.getBestChain().tip()));

  Chain<BlockIndex<BtcBlock>> Achain(0, Atip);
  Chain<BlockIndex<BtcBlock>> Bchain(6, Btip);
  Chain<BlockIndex<BtcBlock>> Cchain(7, Ctip);
  Chain<BlockIndex<BtcBlock>> Dchain(8, Dtip);
  Chain<BlockIndex<BtcBlock>> Echain(7, Etip);
  Chain<BlockIndex<BtcBlock>> Fchain(7, Ftip);

  auto checkBlock = [&](bool shouldBeValid) {
    return [shouldBeValid, &btc](auto&& hash) {
      auto index = btc.getBlockIndex(hash);
      EXPECT_TRUE(index) << "should have found this block";
      if (shouldBeValid) {
        EXPECT_TRUE(index->isValid())
            << "index is expected to be valid, but it isn't";
      } else {
        EXPECT_FALSE(index->isValid())
            << "index is expected to be invalid, but it isn't";
      }
      return index;
    };
  };

  // all blocks from A exist in VALID_BLOCKS
  forEach(Achain, checkBlock(true));
  // all blocks from these chains exist in FAILED_BLOCKS
  forEach(Bchain, checkBlock(false));
  forEach(Cchain, checkBlock(false));
  forEach(Dchain, checkBlock(false));
  forEach(Echain, checkBlock(false));
  forEach(Fchain, checkBlock(false));

  // there's only one chain (no known forks)
  const auto& forkChains = btc.getTips();
  // active + b[5]
  ASSERT_EQ(forkChains.size(), 2);
  ASSERT_TRUE(forkChains.count(Btip->getAncestor(5)));
  ASSERT_TRUE(forkChains.count(Achain.tip()));

  // revalidate subtree at (b) 6
  btc.revalidateSubtree(*Bchain[6], BLOCK_FAILED_BLOCK);
  // all blocks after 6 are valid again
  ASSERT_EQ(btc.getBlocks().size(), 11 + 5 + 3 + 1 + 2 + 1);
  ASSERT_EQ(btc.getTips().size(), 6);

  // remove subtree at (b) 6
  btc.removeSubtree(*Bchain[6]);
  ASSERT_EQ(btc.getBlocks().size(), 6 + 5);
  forEach(Achain, checkBlock(true));
  ASSERT_EQ(btc.getTips().size(), 2);
  ASSERT_TRUE(forkChains.count(B5));
  ASSERT_TRUE(forkChains.count(Achain[9]));
}
