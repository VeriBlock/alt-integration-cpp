// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <memory>

#include "block_headers.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/literals.hpp"

using namespace altintegration;

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
      if(ParseHex(blockHash).empty()) break;
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
  BlockTree<VbkBlock, VbkChainParams> tree(*params);
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
  BlockTree<VbkBlock, VbkChainParams> tree(*params);
  ASSERT_TRUE(tree.bootstrapWithGenesis(state));

  for (size_t i = 1; i < allBlocks.size(); i++) {
    // 28568
    ASSERT_TRUE(tree.acceptBlock(allBlocks[i], state)) << i;
    auto hash = allBlocks[i].getHash();
    index_t* current = tree.getBlockIndex(hash);
    ASSERT_EQ(current->chainWork, cumulativeDifficulties[i]);
  }
}
