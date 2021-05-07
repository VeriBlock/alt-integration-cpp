// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/tree_algo.hpp>

using namespace altintegration;

std::string ToString(const std::unordered_set<BlockIndex<AltBlock>*>& chains) {
  std::ostringstream os;
  os << "size: " << chains.size() << "\n";
  for (const auto& c : chains) {
    os << c->toPrettyString() << "\n";
  }
  return os.str();
}

struct AltInvalidationTest : public ::testing::Test, public PopTestFixture {
  BlockIndex<AltBlock>* tip;
  size_t connId = 0;
  size_t totalInvalidations = 0;

  BlockIndex<AltBlock>*earlier, *earlierChild, *latter, *latterChild;

  AltInvalidationTest() {
    tip = mineAltBlocks(*alttree.getBlocks().begin()->second,
                        10,
                        /*connectBlocks=*/true,
                        /*setState=*/false);
    EXPECT_TRUE(tip->isValid(BLOCK_CONNECTED));

    connId = alttree.connectOnValidityBlockChanged(
        [&](const BlockIndex<AltBlock>&) { totalInvalidations++; });

    // we are going to change the validity of 'earlier'/'latter'
    // and test the status changes of earlierChild/latterChild
    earlier = tip->getAncestor(5);
    earlierChild = tip->getAncestor(7);
    latter = tip->getAncestor(8);
    latterChild = tip->getAncestor(10);
  }

  void ASSERT_EARLIER_VALID() {
    ASSERT_TRUE(earlier->isValid());
    ASSERT_TRUE(earlierChild->isValid());
  }

  void ASSERT_EARLIER_INVALID() {
    ASSERT_FALSE(earlier->isValid());
    ASSERT_FALSE(earlierChild->isValid());
  }

  void ASSERT_LATTER_VALID() {
    ASSERT_TRUE(latter->isValid());
    ASSERT_TRUE(latterChild->isValid());
  };

  void ASSERT_LATTER_INVALID() {
    ASSERT_FALSE(latter->isValid());
    ASSERT_FALSE(latterChild->isValid());
  }
  void ASSERT_ALL_VALID() {
    ASSERT_EARLIER_VALID();
    ASSERT_LATTER_VALID();
  }
  void ASSERT_ALL_INVALID() {
    ASSERT_EARLIER_INVALID();
    ASSERT_LATTER_INVALID();
  }

  template <typename Block, typename F>
  void validateForEach(const Chain<BlockIndex<Block>>& chain, F&& check) {
    for (auto* c : chain) {
      check(c->getHash());
    }
  };
};

TEST_F(AltInvalidationTest, InvalidateBlockInTheMiddleOfChain) {
  // invalidate block at height 5
  auto* toBeInvalidated = tip->getAncestor(5);
  ASSERT_TRUE(toBeInvalidated);

  // backup chain
  Chain<BlockIndex<AltBlock>> chain(0, tip);

  // invalidate block #5
  alttree.invalidateSubtree(*toBeInvalidated, BLOCK_FAILED_BLOCK);

  size_t validBlocks = 0;
  size_t invalidBlocks = 0;
  forEachNodePreorder<AltBlock>(*tip->getAncestor(0),
                                [&](BlockIndex<AltBlock>& b) {
                                  if (b.isValid()) {
                                    validBlocks++;
                                  } else {
                                    invalidBlocks++;
                                  }

                                  return true;
                                });

  // valid block map has 5 blocks (0,1,2,3,4)
  ASSERT_EQ(validBlocks, 5);

  // invalid block map has 6 blocks (5,6,7,8,9,10)
  ASSERT_EQ(invalidBlocks, 6);

  // block #5 is marked as BLOCK_FAILED_BLOCK
  ASSERT_TRUE(chain[5]->getStatus() & BLOCK_FAILED_BLOCK);

  // all next blocks are marked as BLOCK_FAILED_CHILD
  auto* current = chain.next(toBeInvalidated);
  do {
    ASSERT_TRUE(current->getStatus() & BLOCK_FAILED_CHILD);
    current = chain.next(current);
  } while (current != nullptr);

  ASSERT_EQ(totalInvalidations, 6);
}

TEST_F(AltInvalidationTest, MultipleInvalidationsOfTheSameBlock) {
  ASSERT_ALL_VALID();

  // intial invalidation of 'earlier'
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // add one more invalidity reason for 'earlier'
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();

  // remove the initial invalidation reason, the blocks should stay invalid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // remove the initial invalidation reason again, must be a no-op
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // remove the last invalidation reason, the blocks should become valid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_VALID();
}

TEST_F(AltInvalidationTest, OverlappingInvalidations1) {
  ASSERT_ALL_VALID();

  // the earlier block becomes invalid
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();

  // the latter block becomes invalid for multiple reasons
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // the latter block is revalidated, stays invalid
  alttree.revalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.revalidateSubtree(*latter, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // the earlier block is revalidated, all blocks become valid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_VALID();
}

TEST_F(AltInvalidationTest, OverlappingInvalidations2) {
  ASSERT_ALL_VALID();

  // the earlier block becomes invalid
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();

  // the latter block becomes invalid for multiple reasons
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // the earlier block is revalidated and becomes valid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_EARLIER_VALID();
  ASSERT_LATTER_INVALID();

  // the latter block is revalidated, all blocks become valid
  alttree.revalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_EARLIER_VALID();
  ASSERT_LATTER_INVALID();

  alttree.revalidateSubtree(*latter, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_VALID();
}

TEST_F(AltInvalidationTest, OverlappingInvalidations3) {
  ASSERT_ALL_VALID();

  // the latter block becomes invalid
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_EARLIER_VALID();
  ASSERT_LATTER_INVALID();

  // the earlier block becomes invalid for multiple reasons
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // the earlier block is revalidated and becomes valid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_EARLIER_VALID();
  ASSERT_LATTER_INVALID();

  // the latter block is revalidated, all blocks become valid
  alttree.revalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_ALL_VALID();
}

TEST_F(AltInvalidationTest, OverlappingInvalidations4) {
  ASSERT_ALL_VALID();

  // the latter block becomes invalid
  alttree.invalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_EARLIER_VALID();
  ASSERT_LATTER_INVALID();

  // the earlier block becomes invalid for multiple reasons
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.invalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_INVALID();

  // the latter block is revalidated, remains invalid
  alttree.revalidateSubtree(*latter, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();

  // the earlier block is revalidated and becomes valid
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_POP);
  ASSERT_ALL_INVALID();
  alttree.revalidateSubtree(*earlier, BLOCK_FAILED_BLOCK);
  ASSERT_ALL_VALID();
}

TEST_F(AltInvalidationTest, InvalidBlockAsBaseOfMultipleForks) {
  //          /5-6-7-8-9        (a)
  // 0-1-2-3-4-5-6-7-8-9-10     (b)
  //             |\7-8-9        (c)
  //             |  \8-9        (d)
  //             |    \9        (e)
  //             |
  //             |\7-8-9        (f)
  //             |\7-8          (g)
  //              \7            (h)
  // best chain is (b)
  // invalidate block 6 at (b)
  // expect chain  (a) to become new best
  // expect chains (c-h) are completely invalid
  // expect chain  (b) blocks 5-10 to be invalidated
  // expect block 5 from chain (b) to become one of chain tips

  auto* fourth = tip->getAncestor(4);
  auto* sixth = tip->getAncestor(6);
  auto* Atip = mineAltBlocks(*fourth, 5, /*connectBlocks=*/true);
  auto* Btip = tip;
  auto* Ctip = mineAltBlocks(*sixth, 3, /*connectBlocks=*/true);
  auto* Dtip = mineAltBlocks(*Ctip->getAncestor(7), 2, /*connectBlocks=*/true);
  auto* Etip = mineAltBlocks(*Dtip->getAncestor(8), 1, /*connectBlocks=*/true);
  auto* Ftip = mineAltBlocks(*sixth, 3, /*connectBlocks=*/true);  // 7-8-9
  auto* Gtip = mineAltBlocks(*sixth, 2, /*connectBlocks=*/true);  // 7-8
  auto* Htip = mineAltBlocks(*sixth, 1, /*connectBlocks=*/true);  // 7

  ASSERT_EQ(alttree.getNonDeletedBlockCount(), 11 + 5 + 3 + 2 + 1 + 3 + 2 + 1);

  ASSERT_EQ(alttree.getTips().size(), 8);

  // invalidate block (5) on the main chain
  alttree.invalidateSubtree(*sixth, BLOCK_FAILED_BLOCK);

  // chain A is now best
  Chain<BlockIndex<AltBlock>> Achain(0, Atip);
  Chain<BlockIndex<AltBlock>> Bchain(6, Btip);
  Chain<BlockIndex<AltBlock>> Cchain(7, Ctip);
  Chain<BlockIndex<AltBlock>> Dchain(8, Dtip);
  Chain<BlockIndex<AltBlock>> Echain(9, Etip);
  Chain<BlockIndex<AltBlock>> Fchain(7, Ftip);
  Chain<BlockIndex<AltBlock>> Gchain(7, Gtip);
  Chain<BlockIndex<AltBlock>> Hchain(7, Htip);

  auto check = [&](bool shouldBeValid) {
    return [&, shouldBeValid](auto&& hash) {
      auto index = alttree.getBlockIndex(hash);
      EXPECT_TRUE(index);
      if (shouldBeValid) {
        EXPECT_TRUE(index->isValid());
      } else {
        EXPECT_FALSE(index->isValid());
      }
      return index;
    };
  };

  // all blocks from A exist in VALID_BLOCKS
  validateForEach(Achain, check(true));
  // block 5 from (b) still valid
  auto* B5 = Btip->getAncestor(5);
  ASSERT_TRUE(B5);
  ASSERT_TRUE(check(true)(B5->getHash()));
  // all blocks from these chains exist in FAILED_BLOCKS
  validateForEach(Bchain, check(false));
  validateForEach(Cchain, check(false));
  validateForEach(Dchain, check(false));
  validateForEach(Echain, check(false));
  validateForEach(Fchain, check(false));
  validateForEach(Gchain, check(false));
  validateForEach(Hchain, check(false));

  // there's only one chain (no known forks)
  const auto& forkChains = alttree.getTips();
  ASSERT_EQ(forkChains.size(), 2) << ToString(forkChains);
  ASSERT_EQ(forkChains.count(B5), 1);
  ASSERT_EQ(forkChains.count(Atip), 1);

  // remove subtree at (b) 6
  alttree.removeSubtree(*Bchain[6]);

  ASSERT_EQ(alttree.getNonDeletedBlockCount(), 6 + 5);
  validateForEach(Achain, check(true));
  ASSERT_EQ(forkChains.size(), 2);
  ASSERT_TRUE(forkChains.count(B5));
  ASSERT_TRUE(forkChains.count(Achain[9]));

  ASSERT_EQ(totalInvalidations, 5 + 3 + 2 + 1 + 3 + 2 + 1);
}
