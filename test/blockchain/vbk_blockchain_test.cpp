#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include "block_headers.hpp"
#include "util/literals.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"

using namespace altintegration;

struct BlockchainFixture {
  using block_t = VbkBlock;
  using param_t = VbkChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  ValidationState state;

  BtcChainParamsRegTest btcparam;
  VbkChainParamsRegTest vbkparam;
};

struct VbkTestCase {
  std::vector<uint8_t> headers;
  std::shared_ptr<VbkChainParams> params;
  uint32_t startHeight = 0;
  uint32_t offset = 0;

  std::vector<VbkBlock> getBlocks() const {
    std::vector<VbkBlock> ret;
    std::string data;
    std::string in(headers.begin(), headers.end());
    std::istringstream file(in);
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

  VbkBlockTree tree(*value.params, btcparam);

  ASSERT_TRUE(
      tree.bootstrapWithChain(bootstrapChain[0].height, bootstrapChain, state))
      << state.GetPath();
  EXPECT_TRUE(state.IsValid());
  size_t totalBlocks = bootstrapChain.size();

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

  //   tree should be copyable
  auto copy = tree;
  // tree should be copy initializable
  copy = tree;
  // tree should be move initializable
  copy = std::move(tree);
}

INSTANTIATE_TEST_SUITE_P(AcceptBlocksRegression,
                         AcceptTest,
                         testing::ValuesIn(accept_test_cases));
