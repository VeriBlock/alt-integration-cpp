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

struct GetProofTest : public testing::Test {
  using block_t = VbkBlock;
  using param_t = VbkChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  std::shared_ptr<param_t> params;
  ValidationState state;

  std::vector<VbkBlock> allBlocks{};
  std::vector<ArithUint256> cumulativeDifficulties{};

  GetProofTest() {
    params = std::make_shared<VbkChainParamsTest>();
    parseBlocks(generated::vbk_testnet30000);
  }

  void parseBlocks(const std::string& blocks) {
    std::istringstream file(blocks);
    EXPECT_TRUE(!file.fail());
    allBlocks.clear();
    cumulativeDifficulties.clear();
    while (true) {
      std::string blockHash;
      if (!(file >> blockHash)) break;
      std::string cumulDifficulty;
      EXPECT_TRUE(file >> cumulDifficulty);
      std::string blockHeader;
      EXPECT_TRUE(file >> blockHeader);
      VbkBlock block = VbkBlock::fromHex(blockHeader);
      auto difficultyParsed = std::stoull(cumulDifficulty);
      allBlocks.push_back(block);
      cumulativeDifficulties.emplace_back(difficultyParsed);
    }
  }
};

TEST_F(GetProofTest, Blocks100Test) {
  BlockTree<VbkBlock, VbkChainParams> tree(params);
  ASSERT_TRUE(tree.bootstrapWithGenesis(state));

  for (size_t i = 1; i < 101; i++) {
    ASSERT_TRUE(tree.acceptBlock(allBlocks[i], state));
    auto hash = allBlocks[i].getHash();
    index_t* current = tree.getBlockIndex(hash);
    ASSERT_EQ(current->chainWork, cumulativeDifficulties[i]);
  }

  bool ret = tree.acceptBlock(allBlocks[101], state);
  ASSERT_TRUE(ret);
}

TEST_F(GetProofTest, Blocks30kTest) {
  BlockTree<VbkBlock, VbkChainParams> tree(params);
  ASSERT_TRUE(tree.bootstrapWithGenesis(state));

  for (size_t i = 1; i < allBlocks.size(); i++) {
    ASSERT_TRUE(tree.acceptBlock(allBlocks[i], state));
    auto hash = allBlocks[i].getHash();
    index_t* current = tree.getBlockIndex(hash);
    ASSERT_EQ(current->chainWork, cumulativeDifficulties[i]);
  }
}