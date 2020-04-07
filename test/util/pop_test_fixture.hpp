#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/merkle_tree.hpp>
#include <veriblock/mock_miner.hpp>

namespace altintegration {

struct PopTestFixture {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};

  // miners
  MockMiner popminer{};

  // trees
  AltTree alttree = AltTree(altparam, vbkparam, btcparam);

  ValidationState state;

  PopTestFixture() {
    EXPECT_TRUE(popminer.btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(popminer.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.bootstrap(state));
    EXPECT_TRUE(alttree.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.vbk().btc().bootstrapWithGenesis(state));
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
