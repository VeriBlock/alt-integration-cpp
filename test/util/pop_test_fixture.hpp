#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/merkle_tree.hpp>
#include <veriblock/mock_miner.hpp>

namespace altintegration {

// WIP
struct PopTestFixture {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};

  // miners
  MockMiner popminer{};

  // repos
  PayloadsRepositoryInmem<VTB> vtbrepo{};

  // trees
  VbkBlockTree vbktree = VbkBlockTree(
      vbkparam, VbkBlockTree::PopForkComparator{vtbrepo, btcparam, vbkparam});

  ValidationState state;

  PopTestFixture() {
    EXPECT_TRUE(popminer.btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(popminer.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(vbktree.bootstrapWithGenesis(state));
    EXPECT_TRUE(vbktree.btc().bootstrapWithGenesis(state));
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP