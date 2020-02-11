#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include "mock/storage/block_repository_mock.hpp"
#include "mock/storage/cursor_mock.hpp"
#include "util/literals.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
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
  using height_t = BtcBlock::height_t;
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

    EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(true));
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

struct BootstrapTestCase {
  using block_t = BtcBlock;
  using hash_t = BtcBlock::hash_t;
  using height_t = BtcBlock::height_t;

  BtcBlock bootstrap_block;
  height_t height;
  hash_t block_hash;
};

struct BootstrapTest : public testing::TestWithParam<BootstrapTestCase> {
  using block_t = BtcBlock;
  using index_t = typename BlockTree<block_t>::index_t;
  using height_t = typename BlockTree<block_t>::height_t;

  std::shared_ptr<StrictMock<BlockRepositoryMock<index_t>>> repo;
  std::shared_ptr<StrictMock<CursorMock<height_t, index_t>>> cursor;
  ValidationState state;

  BootstrapTest() {
    cursor = std::make_shared<StrictMock<CursorMock<height_t, index_t>>>();
    repo = std::make_shared<StrictMock<BlockRepositoryMock<index_t>>>();

    EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*repo, getCursor()).WillRepeatedly(Return(cursor));
  }
};

static std::vector<BootstrapTestCase> bootstrap_test_cases = {
    // clang-format off
    // mainent genesis block
    {
        BtcBlock::fromRaw("0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49ffff001d1dac2b7c"_unhex),
        0, 
        uint256::fromHex("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f")
    },
    //mainnet 15687 height block
    {
        BtcBlock::fromRaw("01000000d9e9506d9b1800242adae0e97c96f77c86ec8ad3e1cca88a39f5a63b000000008563a346f8466a5d5106797aa54ccfe3829042970d78fa635427335f326c71f2857d1c4affff001d2d6f7258"_unhex),
        15687, 
        uint256::fromHex("000000002f361e7a5c28e1d11bd153830f31081445538c67495bb72b1768ab4a")
    },
    //testnet genesis block
    {
        BtcBlock::fromRaw("0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494dffff001d1aa4ae18"_unhex),
        0, 
        uint256::fromHex("000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943")
    },
    //testnet 12554 height block
    {
        BtcBlock::fromRaw("010000006ed26f617543137b35fd866d8771ce41768110c0fc089d70692e6d1c000000002ed0cde332d6c9d5fea9bf8a6a66375c2cc1d343b4285f5e38ca2aa76e0d3a1d3708c34fc0ff3f1c662b2430"_unhex),
        12554, 
        uint256::fromHex("0000000030f55553e130da0c3f8fac97a2745ae1c742f683037177dd3715460e")
    },
    // clang-format on
};

TEST_P(BootstrapTest, bootstrap_test) {
  auto value = GetParam();

  EXPECT_EQ(value.bootstrap_block.getHash(), value.block_hash);

  EXPECT_CALL(*cursor, seekToFirst()).Times(1);
  EXPECT_CALL(*cursor, isValid()).WillOnce(Return(false));

  BlockTree<BtcBlock> block_chain(repo);
  ASSERT_TRUE(
      block_chain.bootstrap(value.height, value.bootstrap_block, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_EQ(block_chain.getBestChain().tip()->header, value.bootstrap_block);
  EXPECT_EQ(block_chain.getBestChain().bootstrap()->header,
            value.bootstrap_block);
}

INSTANTIATE_TEST_SUITE_P(BootstrapBlocksRegression,
                         BootstrapTest,
                         testing::ValuesIn(bootstrap_test_cases));

struct AcceptTestCase {
  std::string file_name;
};

struct AcceptTest : public testing::TestWithParam<AcceptTestCase> {
  using block_t = BtcBlock;
  using index_t = typename BlockTree<block_t>::index_t;
  using height_t = typename BlockTree<block_t>::height_t;

  std::shared_ptr<StrictMock<BlockRepositoryMock<index_t>>> repo;
  std::shared_ptr<StrictMock<CursorMock<height_t, index_t>>> cursor;
  ValidationState state;
  std::string test_blockheaders_file_path = "../../test/blockchain/";

  AcceptTest() {
    cursor = std::make_shared<StrictMock<CursorMock<height_t, index_t>>>();
    repo = std::make_shared<StrictMock<BlockRepositoryMock<index_t>>>();

    EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*repo, getCursor()).WillRepeatedly(Return(cursor));
  }
};

static std::vector<AcceptTestCase> accept_test_cases = {
    {"btc_blockheaders_mainnet_0_10000"},
    {"btc_blockheaders_mainnet_30000_40000"}};

TEST_P(AcceptTest, accept_test) {
  auto value = GetParam();

  std::ifstream file(test_blockheaders_file_path + value.file_name);
  EXPECT_TRUE(!file.fail());

  uint32_t first_block_height;
  file >> first_block_height;

  std::string temp;
  EXPECT_TRUE(file >> temp);
  BtcBlock bootstrap_block = BtcBlock::fromRaw(ParseHex(temp));

  EXPECT_CALL(*cursor, seekToFirst()).Times(1);
  EXPECT_CALL(*cursor, isValid()).WillOnce(Return(false));

  BlockTree<BtcBlock> block_chain(repo);
  ASSERT_TRUE(
      block_chain.bootstrap(first_block_height, bootstrap_block, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(block_chain.getBestChain().tip()->header, bootstrap_block);
  EXPECT_EQ(block_chain.getBestChain().tip()->height, first_block_height);

  while (file >> temp) {
    BtcBlock block = BtcBlock::fromRaw(ParseHex(temp));

    EXPECT_TRUE(block_chain.acceptBlock(block, state));
    EXPECT_TRUE(state.IsValid());
    EXPECT_EQ(block_chain.getBestChain().tip()->header, block);
    EXPECT_EQ(block_chain.getBestChain().tip()->height, ++first_block_height);
  }

  file.close();
}

INSTANTIATE_TEST_SUITE_P(AcceptBlocksRegression,
                         AcceptTest,
                         testing::ValuesIn(accept_test_cases));
