#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include "block_headers.hpp"
#include "util/literals.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/btc_blockchain_util.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

using namespace VeriBlock;

struct BlockchainFixture {
  using block_t = BtcBlock;
  using param_t = BtcChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  std::shared_ptr<param_t> params;
  std::shared_ptr<BlockRepository<index_t>> repo;
  ValidationState state;

  BlockchainFixture() {
    params = std::make_shared<BtcChainParamsRegTest>();
    repo = std::make_shared<BlockRepositoryInmem<index_t>>();
  }
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
      BtcBlock block = BtcBlock::fromRaw(ParseHex(data));
      ret.push_back(block);
    }
    return ret;
  }
};

struct AcceptTest : public testing::TestWithParam<BtcTestCase>,
                    public BlockchainFixture {};

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

  BlockTree<BtcBlock, BtcChainParams> block_chain(repo, value.params);
  ASSERT_TRUE(
      block_chain.bootstrapWithChain(value.startHeight, bootstrapChain, state))
      << state.GetDebugMessage();
  EXPECT_TRUE(state.IsValid());
  size_t totalBlocks = bootstrapChain.size();

  EXPECT_EQ(block_chain.getBestChain().tip()->header,
            bootstrapChain[bootstrapChain.size() - 1]);
  EXPECT_EQ(block_chain.getBestChain().tip()->height,
            value.startHeight + bootstrapChain.size() - 1);

  for (const auto& block : acceptChain) {
    ASSERT_TRUE(block_chain.acceptBlock(block, state))
        << "block #" << totalBlocks << "\n"
        << "rejection: " << state.GetRejectReason() << ", "
        << "message: " << state.GetDebugMessage();
    EXPECT_TRUE(state.IsValid());
    EXPECT_EQ(block_chain.getBestChain().tip()->header, block);
    EXPECT_EQ(block_chain.getBestChain().tip()->height,
              totalBlocks + value.startHeight);
    ++totalBlocks;
  }
}

INSTANTIATE_TEST_SUITE_P(AcceptBlocksRegression,
                         AcceptTest,
                         testing::ValuesIn(accept_test_cases),
                         [](const testing::TestParamInfo<BtcTestCase>& info) {
                           return format("%d_%snet_startsAt%d",
                                         info.index,
                                         info.param.params->networkName(),
                                         info.param.startHeight);
                         });
