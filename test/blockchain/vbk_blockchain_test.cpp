// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <memory>

#include "block_headers.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/literals.hpp"
#include "veriblock/storage/pop_storage.hpp"

using namespace altintegration;

struct BtcInvalidationTest {
  using block_t = VbkBlock;
  using param_t = VbkChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  ValidationState state;

  BtcChainParamsRegTest btcparam;
  VbkChainParamsRegTest vbkparam;
  PayloadsStorage storage;
};

struct VbkTestCase {
  std::string headers;
  std::shared_ptr<VbkChainParams> params;
  uint32_t startHeight = 0;
  uint32_t offset = 0;

  std::vector<VbkBlock> getBlocks() const {
    std::vector<VbkBlock> ret;
    std::string data;
    std::istringstream file(headers);
    EXPECT_TRUE(!file.fail());
    while (file >> data) {
      auto v = ParseHex(data);
      if (v.empty()) {
        continue;
      }
      ret.push_back(VbkBlock::fromRaw(v));
    }
    return ret;
  }
};

struct AcceptTest : public testing::TestWithParam<VbkTestCase>,
                    public BtcInvalidationTest {};

static std::vector<VbkTestCase> accept_test_cases = {
    /// mainnet
    {generated::vbk_blockheaders_mainnet_200001_230000,
     std::make_shared<VbkChainParamsMain>(),
     200001,
     0},
    {generated::vbk_blockheaders_mainnet_200001_230000,
     std::make_shared<VbkChainParamsMain>(),
     200001,
     3333},

    /// testnet
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     0,
     0},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     0,
     3333},
};

// Read Vbk_blockheaders file.
TEST_P(AcceptTest, BootstrapWithChain) {
  auto value = GetParam();
  auto allblocks = value.getBlocks();

  // skip first `offset` blocks
  allblocks =
      std::vector<VbkBlock>{allblocks.begin() + value.offset, allblocks.end()};

  // make a bootstrap chain by taking first `numBlocksForBootstrap` x2 blocks
  // due to forked blocks we take twice the blocks and expect to collect
  // at least `numBlocksForBootstrap` blocks
  auto bootstrapChain = allblocks;
  bootstrapChain.resize(value.params->numBlocksForBootstrap() * 2);

  ASSERT_GE(allblocks.size(), value.params->numBlocksForBootstrap() * 2);
  // make accept chain - the one that is applied on top of current blocktree
  // state
  std::vector<VbkBlock> acceptChain{
      allblocks.begin() + value.params->numBlocksForBootstrap() * 2,
      allblocks.end()};

  VbkBlockTree tree(*value.params, btcparam, storage);

  ASSERT_TRUE(
      tree.bootstrapWithChain(bootstrapChain[0].height, bootstrapChain, state))
      << state.GetPath();
  EXPECT_TRUE(state.IsValid());
  size_t totalBlocks = bootstrapChain.size();

  ASSERT_TRUE(tree.getBestChain().tip());
  EXPECT_EQ(*tree.getBestChain().tip()->header,
            bootstrapChain[bootstrapChain.size() - 1]);
  EXPECT_EQ(tree.getBestChain().tip()->height,
            bootstrapChain[bootstrapChain.size() - 1].height);

  for (const auto& block : acceptChain) {
    ASSERT_TRUE(tree.acceptBlock(block, state))
        << "block #" << totalBlocks << "\n"
        << "stack trace: " << state.GetPath() << ", "
        << "message: " << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());

    // we are not sure that block gets to the main chain. Make sure it at least
    // exists
    if (*tree.getBestChain().tip()->header != block) {
      EXPECT_NE(tree.getBlockIndex(block.getHash()), nullptr);
    }

    EXPECT_EQ(tree.getBestChain().tip()->height, block.height);
    ++totalBlocks;
  }
}

INSTANTIATE_TEST_SUITE_P(AcceptBlocksRegression,
                         AcceptTest,
                         testing::ValuesIn(accept_test_cases));
