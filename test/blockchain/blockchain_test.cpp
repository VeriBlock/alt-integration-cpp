#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <veriblock/blockchain/blocktree.hpp>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/btc_blockchain_util.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

using namespace VeriBlock;

template <typename TestCase>
struct BlockchainTest : public ::testing::Test {
  using block_t = typename TestCase::block_t;
  using params_base_t = typename TestCase::params_base_t;
  using index_t = typename BlockTree<block_t, params_base_t>::index_t;
  using height_t = typename TestCase::height_t;
  using params_t = typename TestCase::params_t;
  using hash_t = typename block_t::hash_t;

  std::shared_ptr<BlockTree<block_t, params_base_t>> blockchain;
  std::shared_ptr<params_base_t> chainparam;
  std::shared_ptr<Miner<block_t, params_base_t>> miner;

  height_t height = 0;
  ValidationState state;

  BlockchainTest() {
    chainparam = std::make_shared<params_t>();
    miner = std::make_shared<Miner<block_t, params_base_t>>(chainparam);

    blockchain =
        std::make_shared<BlockTree<block_t, params_base_t>>(chainparam);

    // @when
    EXPECT_TRUE(blockchain->bootstrapWithGenesis(state))
        << "bootstrapWithGenesis: " << state.GetRejectReason() << ", "
        << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());
  };
};

TYPED_TEST_SUITE_P(BlockchainTest);

struct BtcTestCase {
  using block_t = BtcBlock;
  using params_base_t = BtcChainParams;
  using params_t = BtcChainParamsRegTest;
  using hash_t = block_t::hash_t;
  using height_t = block_t::height_t;
};

struct VbkTestCase {
  using block_t = VbkBlock;
  using params_base_t = VbkChainParams;
  using params_t = VbkChainParamsRegTest;
  using hash_t = block_t::hash_t;
  using height_t = block_t::height_t;
};

/**
 * Scenario 1
 *
 * @given empty blockchain.
 * @when Bootstrap it with genesis block at height 0 and mine 10000
 * consecutive blocks.
 * @then stored blockchain is valid:
 * - every block has previousHash set correctly
 * - all blocks have same difficulty
 * - timestamp for blocks does not decrease
 * - blocks are statelessly valid
 */
TYPED_TEST_P(BlockchainTest, Scenario1) {
  auto genesis = this->chainparam->getGenesisBlock();

  auto& chain = this->blockchain->getBestChain();
  EXPECT_NE(chain.tip(), nullptr);
  EXPECT_EQ(chain.tip()->height, this->height);
  EXPECT_EQ(chain.size(), 1);
  EXPECT_NE(chain[this->height], nullptr);
  EXPECT_EQ(chain[this->height]->height, this->height);
  EXPECT_EQ(chain[this->height]->header, genesis);

  // mine 5000 blocks
  for (size_t i = 0; i < 5000; i++) {
    auto tip = chain.tip();
    auto block = this->miner->createNextBlock(*tip, {});
    ASSERT_TRUE(checkProofOfWork(block, *this->chainparam));
    ASSERT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetRejectReason();
    std::cout << "block #" << i << "\n";
  }

  // @then
  for (uint32_t i = 1; i < (uint32_t)chain.size(); i++) {
    ASSERT_TRUE(chain[i]);
    ASSERT_TRUE(chain[i - 1]);
    auto prevHash = chain[i]->header.previousBlock;
    auto index = this->blockchain->getBlockIndex(prevHash);
    EXPECT_EQ(index->getHash(), chain[i - 1]->getHash());
    // timestamp is increasing
    EXPECT_GE(chain[i]->header.getBlockTime(), chain[i - 1]->getBlockTime());
    // bits is same for RegTest
    EXPECT_EQ(chain[i]->getDifficulty(), chain[i - 1]->getDifficulty())
        << "different at " << i;
  }
}

/**
 * Scenario 2
 *
 * @given empty blockchain
 * @when bootstrap it with genesis block
 * @and mine 99 blocks in chain A
 * @then chain A is selected
 * @and mine 30 blocks after block 50 in chain B
 * @then best chain is still A, because it is the longest (100 vs 80)
 * @and mine another 30 blocks after block 80 in chain B
 * @then best chain is B, because it is the longest (80+30 vs 100)
 */
TYPED_TEST_P(BlockchainTest, ForkResolutionWorks) {
  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  std::generate_n(
      std::back_inserter(fork1), 100 - 1 /* genesis */, [&]() -> block_t {
        auto* tip = best.tip();
        EXPECT_TRUE(tip);
        auto block = this->miner->createNextBlock(*tip, {});
        EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
            << this->state.GetRejectReason();
        return block;
      });
  // we should be at fork1
  EXPECT_EQ(best.size(), 100);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  // last common block is 49
  fork2.resize(50);
  // mine total 100 new blocks on top of existing 50
  std::generate_n(std::back_inserter(fork2), 100, [&]() {
    // take last block at fork2 and create mine new block on top of that
    auto index = this->blockchain->getBlockIndex(fork2.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = this->miner->createNextBlock(*index, {});
    EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetDebugMessage();
    return block;
  });

  // we should be at fork2
  EXPECT_EQ(best.size(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create 30 blocks at fork1
  std::generate_n(std::back_inserter(fork1), 30, [&]() {
    auto index = this->blockchain->getBlockIndex(fork1.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = this->miner->createNextBlock(*index, {});
    EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetDebugMessage();
    return block;
  });

  // we should be still at fork2
  EXPECT_EQ(best.size(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create another 30 blocks at fork1
  std::generate_n(std::back_inserter(fork1), 30, [&]() {
    auto index = this->blockchain->getBlockIndex(fork1.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = this->miner->createNextBlock(*index, {});
    EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetDebugMessage();
    return block;
  });

  // we should be at fork1
  EXPECT_EQ(best.size(), 160);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_1) {
  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};

  std::generate_n(
      std::back_inserter(fork1), 20 - 1 /* genesis */, [&]() -> block_t {
        auto* tip = best.tip();
        EXPECT_TRUE(tip);
        auto block = this->miner->createNextBlock(*tip, {});
        EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
            << this->state.GetDebugMessage();
        return block;
      });

  EXPECT_EQ(best.size(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::generate_n(std::back_inserter(fork2), 2, [&]() {
    // take last block at fork2 and create mine new block on top of that
    auto index = this->blockchain->getBlockIndex(fork2.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = this->miner->createNextBlock(*index, {});
    EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetDebugMessage();
    return block;
  });

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.size(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 19);
  EXPECT_EQ(best.tip()->getHash(), fork1[18].getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 19);
  EXPECT_EQ(best.tip()->getHash(), fork2[18].getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 18);
  EXPECT_EQ(best.tip()->getHash(), fork2[17].getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 18);
  EXPECT_EQ(best.tip()->getHash(), fork1[17].getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 17);
  EXPECT_EQ(best.tip()->getHash(), fork1[16].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[16].getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best.tip()->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_2) {
  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};

  std::generate_n(
      std::back_inserter(fork1), 20 - 1 /* genesis */, [&]() -> block_t {
        auto* tip = best.tip();
        EXPECT_TRUE(tip);
        auto block = this->miner->createNextBlock(*tip, {});
        EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
            << this->state.GetDebugMessage();
        return block;
      });

  EXPECT_EQ(best.size(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::generate_n(std::back_inserter(fork2), 2, [&]() {
    // take last block at fork2 and create mine new block on top of that
    auto index = this->blockchain->getBlockIndex(fork2.rbegin()->getHash());
    EXPECT_TRUE(index);
    auto block = this->miner->createNextBlock(*index, {});
    EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetDebugMessage();
    return block;
  });

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.size(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  EXPECT_TRUE(this->blockchain->invalidateBlockByHash(best[16]->getHash(),
                                                      this->state));

  EXPECT_EQ(best.size(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(BlockchainTest,
                            Scenario1,
                            ForkResolutionWorks,
                            invalidateTip_test_scenario_1,
                            invalidateTip_test_scenario_2);

// clang-format off
typedef ::testing::Types<
    BtcTestCase,
    VbkTestCase
  > TypesUnderTest;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(BlockchainTestSuite,
                               BlockchainTest,
                               TypesUnderTest);
