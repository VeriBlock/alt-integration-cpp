// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>

using namespace altintegration;

std::string ToString(const std::unordered_set<BlockIndex<AltBlock>*>& chains) {
  std::ostringstream os;
  os << "size: " << chains.size() << "\n";
  for (const auto& c : chains) {
    os << c->toPrettyString() << "\n";
  }
  return os.str();
}

struct BlockchainFixture : public ::testing::Test, public PopTestFixture {
  BlockIndex<AltBlock>* tip;

  BlockchainFixture() {
    tip = mineAltBlocks(*alttree.getValidBlocks().begin()->second, 10);
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

TEST_F(BlockchainFixture, InvalidateBlockInTheMiddleOfChain) {
  // invalidate block at height 5
  auto* toBeInvalidated = tip->getAncestor(5);
  ASSERT_TRUE(toBeInvalidated);

  // backup chain
  Chain<BlockIndex<AltBlock>> chain(0, tip);

  // invalidate block #5
  alttree.invalidateBlockByIndex(*toBeInvalidated);

  // valid block map has 5 blocks (0,1,2,3,4)
  ASSERT_EQ(alttree.getValidBlocks().size(), 5);

  // invalid block map has 6 blocks (5,6,7,8,9,10)
  ASSERT_EQ(alttree.getFailedBlocks().size(), 6);

  // block #5 is marked as BLOCK_FAILED_BLOCK
  ASSERT_TRUE(chain[5]->status & BLOCK_FAILED_BLOCK);

  // all next blocks are marked as BLOCK_FAILED_CHILD
  auto* current = chain.next(toBeInvalidated);
  do {
    ASSERT_TRUE(current->status & BLOCK_FAILED_CHILD);
    current = chain.next(current);
  } while (current != nullptr);
}

TEST_F(BlockchainFixture, InvalidBlockAsBaseOfMultipleForks) {
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
  auto* Atip = mineAltBlocks(*fourth, 5);
  auto* Btip = tip;
  auto* Ctip = mineAltBlocks(*sixth, 3);
  auto* Dtip = mineAltBlocks(*Ctip->getAncestor(7), 2);
  auto* Etip = mineAltBlocks(*Dtip->getAncestor(8), 1);
  auto* Ftip = mineAltBlocks(*sixth, 3);  // 7-8-9
  auto* Gtip = mineAltBlocks(*sixth, 2);  // 7-8
  auto* Htip = mineAltBlocks(*sixth, 1);  // 7

  ASSERT_EQ(alttree.getValidBlocks().size(), 11 + 5 + 3 + 2 + 1 + 3 + 2 + 1);

  // invalidate block (5) on the main chain
  alttree.invalidateBlockByIndex(*sixth);

  // chain A is now best
  Chain<BlockIndex<AltBlock>> Achain(0, Atip);
  Chain<BlockIndex<AltBlock>> Bchain(6, Btip);
  Chain<BlockIndex<AltBlock>> Cchain(7, Ctip);
  Chain<BlockIndex<AltBlock>> Dchain(8, Dtip);
  Chain<BlockIndex<AltBlock>> Echain(9, Etip);
  Chain<BlockIndex<AltBlock>> Fchain(7, Ftip);
  Chain<BlockIndex<AltBlock>> Gchain(7, Gtip);
  Chain<BlockIndex<AltBlock>> Hchain(7, Htip);

  ASSERT_EQ(alttree.getFailedBlocks().size(), 5 + 3 + 2 + 1 + 3 + 2 + 1);

  auto getValidBlockIndex = [&](auto&& hash) {
    auto index = alttree.getBlockIndex(hash);
    EXPECT_TRUE(index);
    EXPECT_TRUE(index->isValid());
    return index;
  };

  auto getFailedBlockIndex = [&](auto&& hash) {
    auto index = alttree.getBlockIndexFailed(hash);
    EXPECT_TRUE(index);
    EXPECT_FALSE(index->isValid());
    return index;
  };

  // all blocks from A exist in VALID_BLOCKS
  forEach(Achain, getValidBlockIndex);
  // block 5 from (b) still valid
  auto* B5 = Btip->getAncestor(5);
  ASSERT_TRUE(B5);
  ASSERT_TRUE(getValidBlockIndex(B5->getHash()));
  // all blocks from these chains exist in FAILED_BLOCKS
  forEach(Bchain, getFailedBlockIndex);
  forEach(Cchain, getFailedBlockIndex);
  forEach(Dchain, getFailedBlockIndex);
  forEach(Echain, getFailedBlockIndex);
  forEach(Fchain, getFailedBlockIndex);
  forEach(Gchain, getFailedBlockIndex);
  forEach(Hchain, getFailedBlockIndex);

  for(const auto& b : alttree.getValidBlocks()) {
    std::cout << b.second->toPrettyString() << "\n";
  }

  // there's only one chain (no known forks)
  const auto& forkChains = alttree.getForkChains();
  ASSERT_EQ(forkChains.size(), 2) << ToString(forkChains);
  ASSERT_EQ(forkChains.count(B5), 1);
  ASSERT_EQ(forkChains.count(Atip), 1);
}