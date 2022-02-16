// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <memory>
#include <veriblock/pop/blockchain/blocktree.hpp>
#include <veriblock/pop/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/pop/bootstraps.hpp>
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/storage/adaptors/block_provider_impl.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>

#include "block_headers.hpp"

using namespace altintegration;

struct GetProofTest : public testing::Test {
  using block_t = VbkBlock;
  using param_t = VbkChainParams;
  using index_t = typename BlockTree<block_t, param_t>::index_t;
  using height_t = typename BlockTree<block_t, param_t>::height_t;
  using hash_t = typename BlockTree<block_t, param_t>::hash_t;

  std::shared_ptr<param_t> params;
  AltChainParamsRegTest altparam;
  ValidationState state;
  adaptors::InmemStorageImpl storage{};
  adaptors::BlockReaderImpl blockProvider{storage, altparam};

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
      if (ParseHex(blockHash).empty()) break;
      std::string cumulDifficulty;
      EXPECT_TRUE(file >> cumulDifficulty);
      std::string blockHeader;
      EXPECT_TRUE(file >> blockHeader);
      VbkBlock block = AssertDeserializeFromHex<VbkBlock>(blockHeader);
      auto difficultyParsed = std::stoull(cumulDifficulty);
      allBlocks.push_back(block);
      cumulativeDifficulties.emplace_back(difficultyParsed);
    }
  }
};

// TODO(warchanT): disabled, because vbk_testnet30000.txt contains pre-progpow
// blocks

TEST_F(GetProofTest, DISABLED_Blocks100Test) {
  BlockTree<VbkBlock, VbkChainParams> tree(*params, blockProvider);
  tree.bootstrapWithGenesis(GetRegTestVbkBlock());

  for (size_t i = 1; i < 101; i++) {
    ASSERT_TRUE(tree.acceptBlockHeader(allBlocks[i], state));
    auto hash = allBlocks[i].getHash();
    index_t* current = tree.getBlockIndex(hash);
    EXPECT_EQ(current->chainWork, cumulativeDifficulties[i]);
  }

  bool ret = tree.acceptBlockHeader(allBlocks[101], state);
  ASSERT_TRUE(ret);
}

TEST_F(GetProofTest, DISABLED_Blocks30kTest) {
  BlockTree<VbkBlock, VbkChainParams> tree(*params, blockProvider);
  tree.bootstrapWithGenesis(GetRegTestVbkBlock());

  for (size_t i = 1; i < allBlocks.size(); i++) {
    // 28568
    ASSERT_TRUE(tree.acceptBlockHeader(allBlocks[i], state)) << i;
    auto hash = allBlocks[i].getHash();
    index_t* current = tree.getBlockIndex(hash);
    ASSERT_EQ(current->chainWork, cumulativeDifficulties[i]);
  }
}
