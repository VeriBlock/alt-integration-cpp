// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <memory>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/time.hpp"

using namespace altintegration;

static std::vector<BlockIndex<VbkBlock>> getChain(int32_t deltaTime,
                                                  int32_t deltaTime_change,
                                                  uint32_t difficulty,
                                                  uint32_t chainlength) {
  assert(chainlength != 0);

  BlockIndex<VbkBlock> blockIndex;
  blockIndex.header = std::make_shared<VbkBlock>();
  blockIndex.header->height = 1;
  blockIndex.header->timestamp = 10000;
  blockIndex.header->difficulty = difficulty;
  blockIndex.height = blockIndex.header->height;
  blockIndex.pprev = nullptr;

  std::vector<BlockIndex<VbkBlock>> chain(chainlength);
  chain[0] = blockIndex;

  for (size_t i = 1; i < chainlength; ++i) {
    if (i > 0 && i % 40 == 0) {
      deltaTime -= deltaTime_change;
    }

    BlockIndex<VbkBlock> temp;
    temp.header = std::make_shared<VbkBlock>();
    temp.header->height = (int32_t)i + 1;
    temp.header->timestamp = chain[i - 1].header->timestamp + deltaTime;
    temp.header->difficulty = difficulty;
    temp.height = (int32_t)i + 1;
    temp.pprev = &chain[i - 1];

    chain[i] = temp;
  }

  return chain;
}

struct VbkBlockchainUtilTest {
  using param_t = VbkChainParams;
  using block_t = VbkBlock;

  std::shared_ptr<param_t> chainparams;
  std::shared_ptr<Miner<block_t, param_t>> miner;

  VbkBlockchainUtilTest() {
    chainparams = std::make_shared<VbkChainParamsMain>();
    miner = std::make_shared<Miner<block_t, param_t>>(*chainparams);
  }
};

struct GetNextWorkRequiredTestCases {
  uint32_t expected_difficulty;
  int32_t deltaTime;
  int32_t deltaTime_change;
  uint32_t chain_difficulty;
  uint32_t chainlength;
};

struct GetNextWorkRequiredTest
    : public testing::TestWithParam<GetNextWorkRequiredTestCases>,
      public VbkBlockchainUtilTest {};

static std::vector<GetNextWorkRequiredTestCases>
    getNextWorkRequired_test_cases = {
        // clang-format off
        {
            ArithUint256::fromHex("000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF").toBits(),
            100,
            0,
            ArithUint256::fromHex("000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF").toBits(),
            VbkChainParamsMain().getRetargetPeriod() - 1
        },
        {
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime(),
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("7FFF80000000000000000000000000000000000000").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() * 2,
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("01FFFE00000000000000000000000000000000000000").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() / 2,
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("09FFF600000000000000000000000000000000000000").toBits(),
            0,
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("02FFFD00000000000000000000000000000000000000").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() / 3,
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("09FFF600000000000000000000000000000000000000").toBits(),
            -10,
            0,
            ArithUint256::fromHex("00FFFF00000000000000000000000000000000000000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("09C23793D191").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() + 10,
            10,
            ArithUint256::fromHex("09184E72A000").toBits(),
            VbkChainParamsMain().getRetargetPeriod()
        },
        {
            ArithUint256::fromHex("08840877ADF0").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() - 10,
            -10,
            ArithUint256::fromHex("09184E72A000").toBits(),
            VbkChainParamsMain().getRetargetPeriod()
        }
        // clang-format on
};

TEST_P(GetNextWorkRequiredTest, getNextWorkRequired_test) {
  auto value = GetParam();

  std::vector<BlockIndex<VbkBlock>> chain = getChain(value.deltaTime,
                                                     value.deltaTime_change,
                                                     value.chain_difficulty,
                                                     value.chainlength);

  uint32_t result =
      getNextWorkRequired(chain[chain.size() - 1], VbkBlock(), *chainparams);

  EXPECT_EQ(value.expected_difficulty, result);
}

INSTANTIATE_TEST_SUITE_P(GetNextWorkRequiredRegression,
                         GetNextWorkRequiredTest,
                         testing::ValuesIn(getNextWorkRequired_test_cases));
struct SingleTest : public ::testing::Test, public VbkBlockchainUtilTest {};

TEST_F(SingleTest, single_test) {
  uint32_t chainlength = chainparams->getRetargetPeriod();

  int32_t deltaTime = chainparams->getTargetBlockTime();

  BlockIndex<VbkBlock> blockIndex;
  blockIndex.header = std::make_shared<VbkBlock>();
  blockIndex.header->height = 1;
  blockIndex.header->timestamp = 10000;
  blockIndex.header->difficulty =
      ArithUint256::fromHex("09184E72A000").toBits();
  blockIndex.height = blockIndex.header->height;
  blockIndex.pprev = nullptr;

  std::vector<BlockIndex<VbkBlock>> chain(chainlength);
  chain[0] = blockIndex;

  for (size_t i = 1; i < chainlength; ++i) {
    if (i >= 60) {
      deltaTime = 300;
    }

    BlockIndex<VbkBlock> temp;
    temp.header = std::make_shared<VbkBlock>();
    temp.header->height = (int32_t)i + 1;
    temp.header->timestamp = chain[i - 1].header->timestamp + deltaTime;
    temp.header->difficulty = ArithUint256::fromHex("09184E72A000").toBits();
    temp.height = (int32_t)i + 1;
    temp.pprev = &chain[i - 1];

    chain[i] = temp;
  }

  uint32_t result =
      getNextWorkRequired(chain[chain.size() - 1], VbkBlock(), *chainparams);

  EXPECT_EQ(ArithUint256::fromHex("0228C35294D0").toBits(), result);
}

TEST(Vbk, CheckBlockTime1) {
  ValidationState state;
  const auto startTime = 1'527'000'000;
  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain;
  for (int i = 0; i < 1000; i++) {
    chain.push_back(std::make_shared<BlockIndex<VbkBlock>>());
    auto& index = chain[chain.size() - 1];
    index->height = VBK_MINIMUM_TIMESTAMP_ONSET_BLOCK_HEIGHT + i;
    index->header = std::make_shared<VbkBlock>();
    index->header->timestamp = startTime + (120 * i);
    index->pprev = i == 0 ? nullptr : chain[i - 1].get();
  }

  VbkBlock block;

  auto& last = chain[chain.size() - 1];
  // validateMinimumTimestampWhenAboveMedian
  block.timestamp = startTime + (120 * 1000);
  ASSERT_TRUE(checkBlockTime(*last, block, state)) << state.GetPath();

  // validateMinimumTimestampWhenBelowMedian
  block.timestamp = 1527118679;
  ASSERT_FALSE(checkBlockTime(*last, block, state)) << state.GetPath();
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-time-too-old");
}

TEST(Vbk, CheckBlockTime2) {
  auto makeBlock = [](int timestamp, int height) -> BlockIndex<VbkBlock> {
    BlockIndex<VbkBlock> index;
    index.height = height;
    index.header = std::make_shared<VbkBlock>();
    index.header->timestamp = timestamp;
    return index;
  };

  ValidationState state;
  std::vector<BlockIndex<VbkBlock>> chain;
  chain.push_back(makeBlock(1527000000, 110000));
  chain.push_back(makeBlock(1527500000, 110001));
  chain.push_back(makeBlock(1528000000, 110002));

  for (size_t i = 0; i < chain.size(); i++) {
    auto& index = chain[i];
    index.pprev = i == 0 ? nullptr : &chain[i - 1];
  }

  VbkBlock block;
  block.timestamp = 1527499999;
  ASSERT_FALSE(checkBlockTime(chain[chain.size() - 1], block, state));
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-time-too-old");

  block.timestamp = 1527500000;
  ASSERT_TRUE(checkBlockTime(chain[chain.size() - 1], block, state));
}

struct BlockchainTest : public ::testing::Test {
  using block_t = VbkBlock;
  using params_base_t = VbkChainParams;
  using params_t = VbkChainParamsRegTest;
  using index_t = typename BlockTree<block_t, params_base_t>::index_t;

  std::shared_ptr<BlockTree<block_t, params_base_t>> blockchain;
  std::shared_ptr<params_base_t> chainparam;
  std::shared_ptr<Miner<block_t, params_base_t>> miner;
  ValidationState state;

  BlockchainTest() {
    chainparam = std::make_shared<params_t>();
    blockchain =
        std::make_shared<BlockTree<block_t, params_base_t>>(*chainparam);

    miner = std::make_shared<Miner<block_t, params_base_t>>(*chainparam);

    // @when
    EXPECT_TRUE(blockchain->bootstrapWithGenesis(state))
        << "bootstrap: " << state.GetPath() << ", " << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());
  };
};

TEST_F(BlockchainTest, InvalidKeystone1) {
  auto& chain = this->blockchain->getBestChain();
  auto block = this->miner->createNextBlock(*chain.tip());
  auto badKeystone = ArithUint256::fromHex("01")
                         .reverse()
                         .template trimLE<VbkBlock::keystone_t::size()>();
  block.previousKeystone = badKeystone;

  ASSERT_FALSE(blockchain->acceptBlock(block, state));
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-bad-keystones");
}

TEST_F(BlockchainTest, InvalidKeystone2) {
  auto& chain = this->blockchain->getBestChain();
  auto block = this->miner->createNextBlock(*chain.tip());
  auto badKeystone = ArithUint256::fromHex("01")
                         .reverse()
                         .template trimLE<VbkBlock::keystone_t::size()>();
  block.secondPreviousKeystone = badKeystone;

  ASSERT_FALSE(blockchain->acceptBlock(block, state));
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-bad-keystones");
}
