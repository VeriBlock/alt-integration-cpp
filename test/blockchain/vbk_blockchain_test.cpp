#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include "block_headers.hpp"
#include "util/literals.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/vbk_blockchain_util.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

using namespace VeriBlock;

struct BlockchainFixture {
  using block_t = VbkBlock;
  using param_t = VbkChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  std::shared_ptr<param_t> params;
  std::shared_ptr<BlockRepository<index_t>> repo;
  ValidationState state;

  BlockchainFixture() {
    params = std::make_shared<VbkChainParamsRegTest>();
    repo = std::make_shared<BlockRepositoryInmem<index_t>>();
  }
};

struct VbkTestCase {
  std::string headers;
  std::shared_ptr<VbkChainParams> params;
  uint32_t startHeight = 0;

  std::vector<VbkBlock> getBlocks() const {
    std::vector<VbkBlock> ret;
    std::string data;
    std::istringstream file(headers);
    EXPECT_TRUE(!file.fail());
    while (file >> data) {
      VbkBlock block = VbkBlock::fromHex(data);
      ret.push_back(block);
    }
    return ret;
  }
};

struct AcceptTest : public testing::TestWithParam<VbkTestCase>,
                    public BlockchainFixture {};

static std::vector<VbkTestCase> accept_test_cases = {
    /// mainnet
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     0},
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     1},
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     99},
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     100},
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     101},
    {generated::vbk_blockheaders_mainnet_0_10000,
     std::make_shared<VbkChainParamsMain>(),
     1337},
    /// testnet
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     0},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     1},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     99},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     100},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     101},
    {generated::vbk_blockheaders_testnet_0_10000,
     std::make_shared<VbkChainParamsTest>(),
     1337},
};

//Read Vbk_blockheaders file.
TEST_P(AcceptTest, BootstrapWithChain) {
  auto value = GetParam();
  auto allblocks = value.getBlocks();
  // skip first `startHeight` blocks
  allblocks = std::vector<VbkBlock>{allblocks.begin() + value.startHeight,
                                    allblocks.end()};

  // make a bootstrap chain by taking first `numBlocksForBootstrap` blocks
  auto bootstrapChain = allblocks;
  bootstrapChain.resize(value.params->numBlocksForBootstrap());

  ASSERT_GE(allblocks.size(), value.params->numBlocksForBootstrap());
  // make accept chain - the one that is applied on top of current blocktree
  // state
  std::vector<VbkBlock> acceptChain{
      allblocks.begin() + value.params->numBlocksForBootstrap(),
      allblocks.end()};

  BlockTree<VbkBlock, VbkChainParams> block_chain(repo, value.params);
  ASSERT_TRUE(block_chain.bootstrapWithChain(
      (int32_t)value.startHeight, bootstrapChain, state))
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
                         [](const testing::TestParamInfo<VbkTestCase>& info) {
                           return format("%d_%snet_startsAt%d",
                                         info.index,
                                         info.param.params->networkName(),
                                         info.param.startHeight);
                         });
