#include <gtest/gtest.h>

#include <veriblock/blockchain/blocktree.hpp>

#include "mock/storage/block_repository_mock.hpp"
#include "mock/storage/cursor_mock.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/btc_blockchain_util.hpp"
#include "veriblock/blockchain/miner.hpp"

using namespace VeriBlock;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;

// eventually this will be type parametrized test
struct BtcBlockchainTest : public ::testing::Test {
  using block_t = BtcBlock;
  using index_t = typename BlockTree<block_t>::index_t;
  using height_t = typename BlockTree<block_t>::height_t;
  using params_t = BtcChainParams;

  std::shared_ptr<StrictMock<BlockRepositoryMock<index_t>>> repo;
  std::shared_ptr<StrictMock<CursorMock<height_t, index_t>>> cursor;
  std::shared_ptr<BlockTree<block_t>> blockchain;

  std::shared_ptr<params_t> chainparam;
  std::shared_ptr<Miner<block_t, params_t>> miner;

  height_t height = 0;
  ValidationState state;

  BtcBlockchainTest() {
    chainparam = std::make_shared<BtcChainParamsRegTest>();
    miner = std::make_shared<Miner<block_t, params_t>>(chainparam);

    cursor = std::make_shared<StrictMock<CursorMock<height_t, index_t>>>();
    repo = std::make_shared<StrictMock<BlockRepositoryMock<index_t>>>();
    blockchain = std::make_shared<BlockTree<block_t>>(repo);

    EXPECT_CALL(*repo, getCursor()).WillRepeatedly(Return(cursor));

    // @given
    auto genesis = chainparam->getGenesisBlock();

    EXPECT_CALL(*repo, put(Field(&index_t::header, genesis)))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*cursor, seekToFirst()).Times(1);
    // database is empty, so first isValid will return false
    EXPECT_CALL(*cursor, isValid()).WillOnce(Return(false));

    // @when
    auto result = blockchain->bootstrap(height, genesis, state);
    EXPECT_TRUE(result);
    EXPECT_TRUE(state.IsValid());
  };
};

/**
 * Scenario 1
 *
 * @given empty blockchain.
 * @when Bootstrap it with BtcRegTest genesis block at height 0 and mine 10000
 * consecutive blocks.
 * @then stored blockchain is valid:
 * - every block has previousHash set correctly
 * - all blocks have same difficulty
 * - timestamp for blocks does not decrease
 * - blocks are statelessly valid
 */
TEST_F(BtcBlockchainTest, Scenario1) {
  auto genesis = chainparam->getGenesisBlock();

  auto& chain = blockchain->getBestChain();
  EXPECT_NE(chain.tip(), nullptr);
  EXPECT_EQ(chain.tip()->height, height);
  EXPECT_EQ(chain.size(), height + 1);
  EXPECT_NE(chain[height], nullptr);
  EXPECT_EQ(chain[height]->height, height);
  EXPECT_EQ(chain[height]->header, genesis);

  EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(false));

  // mine 10000 blocks
  for (size_t i = 0; i < 10000; i++) {
    index_t* tip = chain.tip();
    auto block = miner->createNextBlock(*tip, {});
    ASSERT_TRUE(blockchain->acceptBlock(block, state))
        << state.GetDebugMessage();
  }

  // @then
  for (uint32_t i = 1; i < (uint32_t)chain.size(); i++) {
    ASSERT_TRUE(chain[i]);
    ASSERT_TRUE(chain[i - 1]);
    // corrent previousBlock set
    EXPECT_EQ(chain[i]->header.previousBlock, chain[i - 1]->getHash());
    // timestamp is increasing
    EXPECT_GE(chain[i]->header.timestamp, chain[i - 1]->getBlockTime());
    // bits is same for RegTest
    EXPECT_EQ(chain[i]->getDifficulty(), chain[i - 1]->getDifficulty())
        << "different at " << i;
  }
}

TEST_F(BtcBlockchainTest, ForkResolutionWorks) {
  auto genesis = chainparam->getGenesisBlock();
  EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(false));
  auto& best = blockchain->getBestChain();

  std::vector<BtcBlock> fork1{genesis};
  std::generate_n(
      std::back_inserter(fork1), 100 - 1 /* genesis */, [&]() -> BtcBlock {
        auto* tip = best.tip();
        EXPECT_TRUE(tip);
        auto block = miner->createNextBlock(*tip, {});
        EXPECT_TRUE(blockchain->acceptBlock(block, state))
            << state.GetDebugMessage();
        return block;
      });
  // we should be at fork1
  EXPECT_EQ(best.size(), 100);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<BtcBlock> fork2 = fork1;
  // last common block is 49
  fork2.resize(50);
  // mine total 100 new blocks on top of existing 50
  std::generate_n(std::back_inserter(fork2), 100, [&]() {
    // take last block at fork2 and create mine new block on top of that
    auto index = blockchain->getBlockIndex(fork2.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = miner->createNextBlock(*index, {});
    EXPECT_TRUE(blockchain->acceptBlock(block, state))
        << state.GetDebugMessage();
    return block;
  });

  // we should be at fork2
  EXPECT_EQ(best.size(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create 30 blocks at fork1
  std::generate_n(std::back_inserter(fork1), 30, [&]() {
    auto index = blockchain->getBlockIndex(fork1.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = miner->createNextBlock(*index, {});
    EXPECT_TRUE(blockchain->acceptBlock(block, state))
        << state.GetDebugMessage();
    return block;
  });

  // we should be still at fork2
  EXPECT_EQ(best.size(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create another 30 blocks at fork1
  std::generate_n(std::back_inserter(fork1), 30, [&]() {
    auto index = blockchain->getBlockIndex(fork1.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = miner->createNextBlock(*index, {});
    EXPECT_TRUE(blockchain->acceptBlock(block, state))
        << state.GetDebugMessage();
    return block;
  });

  // we should be at fork1
  EXPECT_EQ(best.size(), 160);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());
}