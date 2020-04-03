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

struct AltTreeTest : public AltTree, public testing::Test {
  ValidationState state;

  AltChainParamsTest altconfig;
  VbkChainParamsRegTest vbkconfig;
  BtcChainParamsRegTest btcconfig;

  AltTreeTest() : AltTree(altconfig, vbkconfig, btcconfig) {
    bootstrapWithGenesis(state);
  }

  std::vector<AltBlock> getTips() {
    std::vector<AltBlock> res;
    for (const auto& tip : chainTips_) {
      res.push_back(tip->header);
    }
    return res;
  }
};

TEST_F(AltTreeTest, acceptBlock_test) {
  AltBlock block1;
  block1.hash = {1, 2, 5};
  block1.previousBlock = altconfig.getBootstrapBlock().hash;
  block1.height = altconfig.getBootstrapBlock().height + 1;
  block1.timestamp = 0;

  acceptBlock(block1, {}, state);
  std::vector<AltBlock> tips = this->getTips();
  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block1) != tips.end());

  AltBlock block2;
  block2.hash = {2, 2, 2};
  block2.previousBlock = altconfig.getBootstrapBlock().hash;
  block2.height = altconfig.getBootstrapBlock().height + 1;
  block2.timestamp = 0;

  acceptBlock(block2, {}, state);

  tips = this->getTips();
  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block1) != tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block2) != tips.end());

  AltBlock block3;
  block3.hash = {3, 2, 3};
  block3.previousBlock = block1.hash;
  block3.height = block1.height + 1;
  block3.timestamp = 0;

  acceptBlock(block3, {}, state);

  tips = this->getTips();
  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block1) == tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block2) != tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block3) != tips.end());

  AltBlock block4;
  block4.hash = {4, 2, 5};
  block4.previousBlock = block2.hash;
  block4.height = block2.height + 1;
  block4.timestamp = 0;

  acceptBlock(block4, {}, state);

  tips = this->getTips();
  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block1) == tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block2) == tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block3) != tips.end());
  EXPECT_TRUE(std::find(tips.begin(), tips.end(), block4) != tips.end());
}
