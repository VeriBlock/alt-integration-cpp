// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/blockchain/vbk_blockchain_util.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <veriblock/bootstraps.hpp>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/time.hpp"

using namespace altintegration;

static std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> getChain(
    int32_t deltaTime,
    int32_t deltaTime_change,
    uint32_t difficulty,
    uint32_t chainlength) {
  assert(chainlength != 0);

  VbkBlock block{};
  block.setHeight(1);
  block.setTimestamp(10000);
  block.setDifficulty(difficulty);
  auto blockIndex = std::make_shared<BlockIndex<VbkBlock>>(nullptr);
  blockIndex->setHeader(block);
  blockIndex->setHeight(block.getHeight());

  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain(chainlength);
  chain[0] = std::move(blockIndex);

  for (size_t i = 1; i < chainlength; ++i) {
    if (i > 0 && i % 40 == 0) {
      deltaTime -= deltaTime_change;
    }

    VbkBlock blockTmp{};
    blockTmp.setHeight((int32_t)i + 1);
    blockTmp.setTimestamp(chain[i - 1]->getHeader().getBlockTime() + deltaTime);
    blockTmp.setDifficulty(difficulty);
    auto temp = std::make_shared<BlockIndex<VbkBlock>>(chain.back().get());
    temp->setHeader(blockTmp);
    temp->setHeight(blockTmp.getHeight());

    chain[i] = std::move(temp);
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

  auto chain = getChain(value.deltaTime,
                        value.deltaTime_change,
                        value.chain_difficulty,
                        value.chainlength);

  uint32_t result =
      getNextWorkRequired(*chain[chain.size() - 1], VbkBlock(), *chainparams);

  EXPECT_EQ(value.expected_difficulty, result);
}

INSTANTIATE_TEST_SUITE_P(GetNextWorkRequiredRegression,
                         GetNextWorkRequiredTest,
                         testing::ValuesIn(getNextWorkRequired_test_cases));
struct SingleTest : public ::testing::Test, public VbkBlockchainUtilTest {};

TEST_F(SingleTest, single_test) {
  uint32_t chainlength = chainparams->getRetargetPeriod();

  int32_t deltaTime = chainparams->getTargetBlockTime();

  VbkBlock block{};
  block.setHeight(1);
  block.setTimestamp(10000);
  block.setDifficulty(ArithUint256::fromHex("09184E72A000").toBits());
  auto blockIndex = std::make_shared<BlockIndex<VbkBlock>>(nullptr);
  blockIndex->setHeader(block);
  blockIndex->setHeight(block.getHeight());
  blockIndex->pprev = nullptr;

  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain(chainlength);
  chain[0] = std::move(blockIndex);

  for (size_t i = 1; i < chainlength; ++i) {
    if (i >= 60) {
      deltaTime = 300;
    }

    VbkBlock blockTmp{};
    blockTmp.setHeight((int32_t)i + 1);
    blockTmp.setTimestamp(chain[i - 1]->getHeader().getBlockTime() + deltaTime);
    blockTmp.setDifficulty(ArithUint256::fromHex("09184E72A000").toBits());
    auto temp = std::make_shared<BlockIndex<VbkBlock>>(chain[i - 1].get());
    temp->setHeader(blockTmp);
    temp->setHeight(blockTmp.getHeight());

    chain[i] = std::move(temp);
  }

  uint32_t result =
      getNextWorkRequired(*chain.back(), VbkBlock(), *chainparams);

  EXPECT_EQ(ArithUint256::fromHex("0228C35294D0").toBits(), result);
}

TEST(Vbk, CheckBlockTime1) {
  ValidationState state;
  const auto startTime = 1'527'000'000;
  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain;
  for (int i = 0; i < 1000; i++) {
    chain.push_back(std::make_shared<BlockIndex<VbkBlock>>(chain.back().get()));
    auto& index = chain.back();
    VbkBlock blockTmp{};
    blockTmp.setTimestamp(startTime + (120 * i));
    index->setHeight(VBK_MINIMUM_TIMESTAMP_ONSET_BLOCK_HEIGHT + i);
    index->setHeader(blockTmp);
  }

  VbkChainParamsMain main;
  VbkBlock block{};

  auto& last = chain[chain.size() - 1];
  // validateMinimumTimestampWhenAboveMedian
  block.setTimestamp(startTime + (120 * 1000));
  bool r1 = checkBlockTime<VbkBlock, VbkChainParams>(*last, block, state, main);
  ASSERT_TRUE(r1) << state.GetPath();

  // validateMinimumTimestampWhenBelowMedian
  block.setTimestamp(1527118679);
  bool r2 = checkBlockTime<VbkBlock, VbkChainParams>(*last, block, state, main);
  ASSERT_FALSE(r2) << state.GetPath();
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-time-too-old");
}

TEST(Vbk, CheckBlockTime2) {
  auto makeBlock = [](BlockIndex<VbkBlock>* prev,
                      int timestamp,
                      int height) -> std::shared_ptr<BlockIndex<VbkBlock>> {
    auto index = std::make_shared<BlockIndex<VbkBlock>>(prev);
    VbkBlock blockTmp{};
    blockTmp.setTimestamp(timestamp);
    index->setHeight(height);
    index->setHeader(blockTmp);
    return index;
  };

  ValidationState state;
  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain;
  chain.push_back(makeBlock(nullptr, 1527000000, 110000));
  chain.push_back(makeBlock(chain.back().get(), 1527500000, 110001));
  chain.push_back(makeBlock(chain.back().get(), 1528000000, 110002));

  VbkBlock block;
  VbkChainParamsMain params;
  block.setTimestamp(1527499999);
  bool r1 = checkBlockTime<VbkBlock, VbkChainParams>(
      *chain[chain.size() - 1], block, state, params);
  ASSERT_FALSE(r1);
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-time-too-old");

  block.setTimestamp(1527500000);
  bool r2 = checkBlockTime<VbkBlock, VbkChainParams>(
      *chain[chain.size() - 1], block, state, params);
  ASSERT_TRUE(r2);
}

template <typename T>
T getGenesisBlockHelper();

template <>
VbkBlock getGenesisBlockHelper() {
  return GetRegTestVbkBlock();
}
template <>
BtcBlock getGenesisBlockHelper() {
  return GetRegTestBtcBlock();
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
    EXPECT_TRUE(blockchain->bootstrapWithGenesis(
        getGenesisBlockHelper<block_t>(), state))
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
  block.setPreviousKeystone(badKeystone);
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
  block.setSecondPreviousKeystone(badKeystone);
  ASSERT_FALSE(blockchain->acceptBlock(block, state));
  ASSERT_EQ(state.GetPathParts()[state.GetPathParts().size() - 1],
            "vbk-bad-keystones");
}
