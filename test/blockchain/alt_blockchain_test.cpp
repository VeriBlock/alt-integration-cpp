#include <gtest/gtest.h>

#include <algorithm>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"

using namespace altintegration;

struct AltTreeTest : public AltTree, public testing::Test {
  AltChainParams config;

  ValidationState state;

  AltTreeTest() : AltTree(config) { bootstrapWithGenesis(state); }

  std::vector<AltBlock> getTips() {
    std::vector<AltBlock> res;
    for (const auto& tip : chainTips_) {
      res.push_back(tip->header);
    }
    return res;
  }
};

TEST_F(AltTreeTest, acceptBlock_test) {
  Payloads temp;

  AltBlock block1;
  block1.hash = {1, 2, 5};
  block1.previousBlock = config.getGenesisBlock().hash;
  block1.height = config.getGenesisBlock().height + 1;
  block1.timestamp = 0;

  acceptBlock(block1, temp, state);

  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block1) != this->getTips().end());

  AltBlock block2;
  block1.hash = {2, 2, 2};
  block1.previousBlock = config.getGenesisBlock().hash;
  block1.height = config.getGenesisBlock().height + 1;
  block1.timestamp = 0;

  acceptBlock(block2, temp, state);

  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block1) != this->getTips().end());

  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block2) != this->getTips().end());

  AltBlock block3;
  block1.hash = {3, 2, 3};
  block1.previousBlock = block1.hash;
  block1.height = block1.height + 1;
  block1.timestamp = 0;

  acceptBlock(block3, temp, state);

  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block1) == this->getTips().end());

  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block2) != this->getTips().end());

  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block3) != this->getTips().end());

  AltBlock block4;
  block1.hash = {4, 2, 5};
  block1.previousBlock = block2.hash;
  block1.height = block2.height + 1;
  block1.timestamp = 0;

  acceptBlock(block4, temp, state);

  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block1) == this->getTips().end());

  EXPECT_TRUE(state.IsValid());
  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block2) == this->getTips().end());

  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block3) != this->getTips().end());

  EXPECT_TRUE(std::find(this->getTips().begin(),
                        this->getTips().end(),
                        block4) != this->getTips().end());
}
