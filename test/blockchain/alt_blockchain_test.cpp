#include <gtest/gtest.h>

#include <algorithm>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

struct AltTreeTest : public testing::Test {
  ValidationState state;

  AltChainParamsTest altconfig{};
  VbkChainParamsRegTest vbkconfig{};
  BtcChainParamsRegTest btcconfig{};
  AltTree altTree;

  AltTreeTest() : altTree(altconfig, vbkconfig, btcconfig) {
    altTree.bootstrapWithGenesis(state);
  }
};

TEST_F(AltTreeTest, acceptBlock_test) { ASSERT_TRUE(true); }
