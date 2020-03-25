#include <gtest/gtest.h>

#include <veriblock/blockchain/blocktree.hpp>

#include "util/visualize.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/btc_blockchain_util.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"
#include "veriblock/time.hpp"

using namespace altintegration;

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
    miner = std::make_shared<Miner<block_t, params_base_t>>(*chainparam);

    blockchain =
        std::make_shared<BlockTree<block_t, params_base_t>>(*chainparam);

    // @when
    EXPECT_TRUE(blockchain->bootstrapWithGenesis(state))
        << "bootstrapWithGenesis: " << state.GetRejectReason() << ", "
        << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());
  };

  void addToFork(std::vector<block_t>& fork, int size) {
    std::generate_n(std::back_inserter(fork), size, [this, &fork]() -> block_t {
      auto hash = fork.rbegin()->getHash();
      auto* index = this->blockchain->getBlockIndex(hash);
      EXPECT_TRUE(index);
      auto block = this->miner->createNextBlock(*index);
      EXPECT_TRUE(this->blockchain->acceptBlock(block, this->state))
          << this->state.GetDebugMessage();
      return block;
    });
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
  EXPECT_EQ(chain.chainHeight(), 0);
  EXPECT_NE(chain[this->height], nullptr);
  EXPECT_EQ(chain[this->height]->height, this->height);
  EXPECT_EQ(chain[this->height]->header, genesis);

  // mine 5000 blocks
  for (size_t i = 0; i < 5000; i++) {
    auto tip = chain.tip();
    auto block = this->miner->createNextBlock(*tip);
    ASSERT_TRUE(checkProofOfWork(block, *this->chainparam));
    ASSERT_TRUE(this->blockchain->acceptBlock(block, this->state))
        << this->state.GetRejectReason();
  }

  // @then
  for (uint32_t i = 1; i <= (uint32_t)chain.chainHeight(); i++) {
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

  this->addToFork(fork1, 100 - 1 /*genesis*/);

  // we should be at fork1
  EXPECT_EQ(best.blocksCount(), 100);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  // last common block is 49
  fork2.resize(50);
  // mine total 100 new blocks on top of existing 50
  this->addToFork(fork2, 100);

  // we should be at fork2
  EXPECT_EQ(best.blocksCount(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create 30 blocks at fork1
  this->addToFork(fork1, 30);

  // we should be still at fork2
  EXPECT_EQ(best.blocksCount(), 150);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // create another 30 blocks at fork1
  this->addToFork(fork1, 30);

  // we should be at fork1
  EXPECT_EQ(best.blocksCount(), 160);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_1) {
  // In this test is considered such case
  //                / D - E - Z (tip)
  //  ... - A - B - C
  //                \ F - G
  // Blocks are removed consistently so expect behaviour is described below

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 20 - 1 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);

  this->addToFork(fork2, 2);

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'Z'
  //                / D - E (tip)
  //  ... - A - B - C
  //                \ F - G
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 19);
  EXPECT_EQ(best.tip()->getHash(), fork1[18].getHash());

  // remove block 'E'
  //                / D
  //  ... - A - B - C
  //                \ F - G (tip)
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 19);
  EXPECT_EQ(best.tip()->getHash(), fork2[18].getHash());

  // remove block 'G'
  //                / D
  //  ... - A - B - C
  //                \ F (tip)
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 18);
  EXPECT_EQ(best.tip()->getHash(), fork2[17].getHash());

  // remove block 'F'
  //                / D (tip)
  //  ... - A - B - C
  //
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 18);
  EXPECT_EQ(best.tip()->getHash(), fork1[17].getHash());

  // remove block 'D'
  //
  //  ... - A - B - C (tip)
  //
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 17);
  EXPECT_EQ(best.tip()->getHash(), fork1[16].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[16].getHash());

  // remove block 'C'
  //
  //  ... - A - B (tip)
  //
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_2) {
  // In this test considered this case
  //                / D - E - Z (tip)
  //  ... - A - B - C
  //                \ F - G
  // Remove block C. Expect C, D, E, Z, F, G to be removed.

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 20 - 1 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 2);

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'C' and chain above this block
  this->blockchain->invalidateBlockByHash(best[16]->getHash());

  EXPECT_EQ(best.blocksCount(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_3) {
  // In this test considered this case
  //                / D - E - Z (tip)
  //  ... - A - B - C
  //                \ F - G
  // Remove blocks F, Z. Expect F, G, Z to be removed. Tip is at E.

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 20 - 1 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 2);

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'F' and chain above this block
  this->blockchain->invalidateBlockByHash(fork2[fork2.size() - 2].getHash());

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'Z'
  this->blockchain->invalidateBlockByHash(best.tip()->getHash());

  EXPECT_EQ(best.blocksCount(), 19);
  EXPECT_EQ(best.tip()->getHash(), fork1[18].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_4) {
  // In this test considered this case
  //                      / H - I
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F - G
  // Remove block C. Expect C, D, E, Z, F, G, P, R, H, I to be removed.

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 2);

  std::vector<block_t> fork3 = fork1;
  fork3.resize(19);
  this->addToFork(fork3, 2);

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(fork3.size(), 21);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'C' and chain above this block
  this->blockchain->invalidateBlockByHash(best[16]->getHash());

  EXPECT_EQ(best.blocksCount(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork3[15].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_5) {
  // In this test considered this case
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q
  // Remove block C. Expect C, D, E, Z, F, G, P, R, H, I, Q to be removed.

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 4);

  std::vector<block_t> fork3 = fork2;
  fork3.resize(19);
  this->addToFork(fork3, 1);

  EXPECT_EQ(fork2.size(), 21);
  EXPECT_EQ(fork3.size(), 20);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'C' and chain above this block
  this->blockchain->invalidateBlockByHash(best[16]->getHash());

  EXPECT_EQ(best.blocksCount(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork2[15].getHash());
  EXPECT_EQ(best.tip()->getHash(), fork3[15].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_6) {
  // In this test considered this case
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q
  // Step 1
  // remove block G with blocks H, I, Q
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F
  // Step 2
  // remove block D with blocks E, Z, P, R
  //  ... - A - B - C
  //                \ F (tip)

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 4);

  std::vector<block_t> fork3 = fork2;
  fork3.resize(19);
  this->addToFork(fork3, 1);

  EXPECT_EQ(fork2.size(), 21);
  EXPECT_EQ(fork3.size(), 20);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'G' and chain above this block
  this->blockchain->invalidateBlockByHash(fork2[18].getHash());

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'D' and chain above this block
  this->blockchain->invalidateBlockByHash(fork1[17].getHash());

  EXPECT_EQ(best.blocksCount(), 18);
  EXPECT_EQ(best.tip()->getHash(), fork2[17].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_7) {
  // In this test considered this case
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q
  // Step 1
  // remove block F with blocks G, H, I, Q
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  // Step 2
  // remove block D with blocks E, Z, P, R
  //
  //  ... - A - B - C (tip)

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 4);

  std::vector<block_t> fork3 = fork2;
  fork3.resize(19);
  this->addToFork(fork3, 1);

  EXPECT_EQ(fork2.size(), 21);
  EXPECT_EQ(fork3.size(), 20);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'F' and chain above this block
  this->blockchain->invalidateBlockByHash(fork2[17].getHash());

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'D' and chain above this block
  this->blockchain->invalidateBlockByHash(fork1[17].getHash());

  EXPECT_EQ(best.blocksCount(), 17);
  EXPECT_EQ(best.tip()->getHash(), fork1[16].getHash());
}

TYPED_TEST_P(BlockchainTest, invalidateTip_test_scenario_8) {
  // In this test considered this case
  //                / Q - S - Y (fork2)
  //  ... - A - B - C - D - E - Z - P - R (tip)
  //                \ F - G - H (fork1)
  //
  // Step 1
  // remove block C with blocks D, E, Z, P, R and fork1, fork2
  //
  //  ... - A - B (tip)

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 3);

  std::vector<block_t> fork3 = fork1;
  fork3.resize(17);
  this->addToFork(fork3, 3);

  EXPECT_EQ(fork2.size(), 20);
  EXPECT_EQ(fork3.size(), 20);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // remove block 'B' and chain above this block
  this->blockchain->invalidateBlockByHash(fork1[16].getHash());

  EXPECT_EQ(best.blocksCount(), 16);
  EXPECT_EQ(best.tip()->getHash(), fork1[15].getHash());
}

TYPED_TEST_P(BlockchainTest, acceptBlock_test_scenario_1) {
  // In this test considered this case
  //                / D - E - Z (tip)
  //  ... - A - B - C
  //                \ F - G
  // We will add two blocks to the fork (F, G) so the expect behaviour is
  // described below
  // Step 2
  //                / D - E - Z
  //  ... - A - B - C
  //                \ F - G - H - I (tip)
  // After that we will remove block 'F' to check that it will correctly switch
  // to the main chain Step 3
  //                / D - E - Z (tip)
  //  ... - A - B - C
  //

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 20 - 1 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 2);

  EXPECT_EQ(fork2.size(), 19);
  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // step 2
  // Add blocks 'H' and 'I'
  this->addToFork(fork2, 2);

  EXPECT_EQ(fork2.size(), 21);
  EXPECT_EQ(best.blocksCount(), 21);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());

  // step 3
  // remove block 'F'
  this->blockchain->invalidateBlockByHash(fork2[17].getHash());

  EXPECT_EQ(best.blocksCount(), 20);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());
}

TYPED_TEST_P(BlockchainTest, acceptBlock_test_scenario_2) {
  // In this test considered this case
  //                / D - E - Z - P - R (tip)
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q
  // Step 2
  // add 4 blocks to the fork with the tip 'Q'
  //                / D - E - Z - P - R
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q - Y - U - M - N (tip)
  // Step 3
  // remove block 'D' and blocks E, Z, P, R
  //
  //  ... - A - B - C
  //                \ F - G - H - I
  //                      \ Q - Y - U - M - N (tip)
  // Step 4
  // remove block 'Q' and blocks Y, U, M, N
  //
  //  ... - A - B - C
  //                \ F - G - H - I (tip)

  using block_t = typename TypeParam::block_t;

  auto genesis = this->chainparam->getGenesisBlock();
  auto& best = this->blockchain->getBestChain();

  std::vector<block_t> fork1{genesis};
  this->addToFork(fork1, 21 /*genesis*/);

  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  std::vector<block_t> fork2 = fork1;
  fork2.resize(17);
  this->addToFork(fork2, 4);

  std::vector<block_t> fork3 = fork2;
  fork3.resize(19);
  this->addToFork(fork3, 1);

  EXPECT_EQ(fork2.size(), 21);
  EXPECT_EQ(fork3.size(), 20);
  EXPECT_EQ(best.blocksCount(), 22);
  EXPECT_EQ(best.tip()->getHash(), fork1.rbegin()->getHash());

  // add blocks Y, U, M, N
  this->addToFork(fork3, 4);

  EXPECT_EQ(fork3.size(), 24);
  EXPECT_EQ(best.blocksCount(), 24);
  EXPECT_EQ(best.tip()->getHash(), fork3.rbegin()->getHash());

  // remove block D
  this->blockchain->invalidateBlockByHash(fork1[17].getHash());

  EXPECT_EQ(best.blocksCount(), 24);
  EXPECT_EQ(best.tip()->getHash(), fork3.rbegin()->getHash());

  // remove block Q
  this->blockchain->invalidateBlockByHash(fork3[19].getHash());

  EXPECT_EQ(best.blocksCount(), 21);
  EXPECT_EQ(best.tip()->getHash(), fork2.rbegin()->getHash());
}

// make sure to enumerate the test cases here
REGISTER_TYPED_TEST_SUITE_P(BlockchainTest,
                            Scenario1,
                            ForkResolutionWorks,
                            invalidateTip_test_scenario_1,
                            invalidateTip_test_scenario_2,
                            invalidateTip_test_scenario_3,
                            invalidateTip_test_scenario_4,
                            invalidateTip_test_scenario_5,
                            invalidateTip_test_scenario_6,
                            invalidateTip_test_scenario_7,
                            invalidateTip_test_scenario_8,
                            acceptBlock_test_scenario_1,
                            acceptBlock_test_scenario_2);

// clang-format off
typedef ::testing::Types<
    BtcTestCase,
    VbkTestCase
  > TypesUnderTest;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(BlockchainTestSuite,
                               BlockchainTest,
                               TypesUnderTest);
