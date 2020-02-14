#include <gtest/gtest.h>

#include <memory>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"

using namespace VeriBlock;

static std::vector<BlockIndex<VbkBlock>> getChain(int32_t deltaTime,
                                                  int32_t deltaTime_change,
                                                  const uint32_t& difficulty,
                                                  const uint32_t& chainlength) {
  BlockIndex<VbkBlock> blockIndex;
  blockIndex.header.height = 1;
  blockIndex.header.timestamp = 10000;
  blockIndex.header.difficulty = difficulty;
  blockIndex.height = blockIndex.header.height;
  blockIndex.pprev = nullptr;

  std::vector<BlockIndex<VbkBlock>> chain(chainlength);
  chain[0] = blockIndex;

  for (size_t i = 1; i < chainlength; ++i) {
    if (i > 0 && i % 40 == 0) {
      deltaTime -= deltaTime_change;
    }

    BlockIndex<VbkBlock> temp;
    temp.header.height = (int32_t)i + 1;
    temp.header.timestamp = chain[i - 1].header.timestamp + deltaTime;
    temp.header.difficulty = difficulty;
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
    miner = std::make_shared<Miner<block_t, param_t>>(chainparams);
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
            VbkChainParamsMain().getRetargetPeriod() + 1
        },
        {
            ArithUint256::fromHex("08840877ADF0").toBits(),
            (int32_t) VbkChainParamsMain().getTargetBlockTime() - 10,
            -10,
            ArithUint256::fromHex("09184E72A000").toBits(),
            VbkChainParamsMain().getRetargetPeriod() + 1
        }
        // clang-format on
};

TEST_P(GetNextWorkRequiredTest, getNextWorkRequired_test) {
  auto value = GetParam();

  std::vector<BlockIndex<VbkBlock>> chain = getChain(value.deltaTime,
                                                     value.deltaTime_change,
                                                     value.chain_difficulty,
                                                     value.chainlength);

  uint32_t result = getNextWorkRequired(
      chain[chain.size() - 2], chain[chain.size() - 1].header, *chainparams);

  EXPECT_EQ(value.expected_difficulty, result);
}

INSTANTIATE_TEST_SUITE_P(GetNextWorkRequiredRegression,
                         GetNextWorkRequiredTest,
                         testing::ValuesIn(getNextWorkRequired_test_cases));
struct SingleTest : public ::testing::Test, public VbkBlockchainUtilTest {};

TEST_F(SingleTest, single_test) {
  uint32_t chainlength = chainparams->getRetargetPeriod() + 1;

  int32_t deltaTime = chainparams->getTargetBlockTime();

  BlockIndex<VbkBlock> blockIndex;
  blockIndex.header.height = 1;
  blockIndex.header.timestamp = 10000;
  blockIndex.header.difficulty = ArithUint256::fromHex("09184E72A000").toBits();
  blockIndex.height = blockIndex.header.height;
  blockIndex.pprev = nullptr;

  std::vector<BlockIndex<VbkBlock>> chain(chainlength);
  chain[0] = blockIndex;

  for (size_t i = 1; i < chainlength; ++i) {
    if (i >= 60) {
      deltaTime = 300;
    }

    BlockIndex<VbkBlock> temp;
    temp.header.height = (int32_t)i + 1;
    temp.header.timestamp = chain[i - 1].header.timestamp + deltaTime;
    temp.header.difficulty = ArithUint256::fromHex("09184E72A000").toBits();
    temp.height = (int32_t)i + 1;
    temp.pprev = &chain[i - 1];

    chain[i] = temp;
  }

  uint32_t result = getNextWorkRequired(
      chain[chain.size() - 2], chain[chain.size() - 1].header, *chainparams);

  EXPECT_EQ(ArithUint256::fromHex("0228C35294D0").toBits(), result);
}
