// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <fstream>
#include <memory>
#include <veriblock/pop/blockchain/blocktree.hpp>
#include <veriblock/pop/blockchain/btc_blockchain_util.hpp>
#include <veriblock/pop/blockchain/miner.hpp>
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/storage/adaptors/block_provider_impl.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>

#include "block_headers.hpp"

using namespace altintegration;

struct BtcInvalidationTest {
  using block_t = BtcBlock;
  using param_t = BtcChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  std::shared_ptr<param_t> params;
  AltChainParamsRegTest altparam{};
  ValidationState state;
  adaptors::InmemStorageImpl storage{};
  adaptors::BlockReaderImpl blockProvider{storage, altparam};

  BtcInvalidationTest() { params = std::make_shared<BtcChainParamsRegTest>(); }
};

struct BtcTestCase {
  std::string headers;
  std::shared_ptr<BtcChainParams> params;
  uint32_t startHeight = 0;

  std::vector<BtcBlock> getBlocks() const {
    std::vector<BtcBlock> ret;
    std::string data;
    std::istringstream file(headers);
    EXPECT_TRUE(!file.fail());
    while (file >> data) {
      auto v = ParseHex(data);
      if (v.empty()) {
        continue;
      }
      ret.push_back(AssertDeserializeFromRaw<BtcBlock>(v));
    }
    return ret;
  }
};

struct AcceptTest : public testing::TestWithParam<BtcTestCase>,
                    public BtcInvalidationTest {};

static std::vector<BtcTestCase> accept_test_cases = {
    /// mainnet
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     0},
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     1},
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     2015},
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     2016},
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     2017},
    {generated::btc_blockheaders_mainnet_0_10000,
     std::make_shared<BtcChainParamsMain>(),
     5000},
    /// testnet
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     0},
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     1},
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     2015},
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     2016},
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     2017},
    {generated::btc_blockheaders_testnet_0_10000,
     std::make_shared<BtcChainParamsTest>(),
     5000},
};

/**
 * Read btc_blockheaders file.
 */
TEST_P(AcceptTest, BootstrapWithChain) {
  auto value = GetParam();
  auto allblocks = value.getBlocks();
  // skip first `startHeight` blocks
  allblocks = std::vector<BtcBlock>{allblocks.begin() + value.startHeight,
                                    allblocks.end()};

  // make a bootstrap chain by taking first `numBlocksForBootstrap` blocks
  auto bootstrapChain = allblocks;
  bootstrapChain.resize(value.params->numBlocksForBootstrap());

  ASSERT_GE(allblocks.size(), value.params->numBlocksForBootstrap());
  // make accept chain - the one that is applied on top of current blocktree
  // state
  std::vector<BtcBlock> acceptChain{
      allblocks.begin() + value.params->numBlocksForBootstrap(),
      allblocks.end()};

  BlockTree<BtcBlock, BtcChainParams> tree(*value.params, blockProvider);
  ASSERT_NO_FATAL_FAILURE(tree.bootstrapWithChain(value.startHeight, bootstrapChain));
  size_t totalBlocks = bootstrapChain.size();

  EXPECT_EQ(tree.getBestChain().tip()->getHeader(),
            bootstrapChain[bootstrapChain.size() - 1]);
  EXPECT_EQ(tree.getBestChain().tip()->getHeight(),
            value.startHeight + bootstrapChain.size() - 1);

  for (const auto& block : acceptChain) {
    ASSERT_TRUE(tree.acceptBlockHeader(block, state))
        << "block #" << totalBlocks << "\n"
        << "stack trace: " << state.GetPath() << ", "
        << "message: " << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());
    EXPECT_EQ(tree.getBestChain().tip()->getHeader(), block);
    EXPECT_EQ(tree.getBestChain().tip()->getHeight(),
              totalBlocks + value.startHeight);
    ++totalBlocks;
  }
}

INSTANTIATE_TEST_SUITE_P(AcceptBlocksRegression,
                         AcceptTest,
                         testing::ValuesIn(accept_test_cases));
