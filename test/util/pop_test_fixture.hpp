#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include "util/test_utils.hpp"
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
    EXPECT_TRUE(alttree.bootstrap(state));
    EXPECT_TRUE(alttree.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.vbk().btc().bootstrapWithGenesis(state));
  }

  void mineAltBlocks(uint32_t num, std::vector<AltBlock>& chain) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(alttree.acceptBlock(*chain.rbegin(), state));
      ASSERT_TRUE(state.IsValid());
    }
  }

  PublicationData generatePublicationData(const AltBlock& block) {
    const static std::vector<uint8_t> payout_info = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    PublicationData pubData;
    pubData.payoutInfo = payout_info;
    pubData.identifier = 0;
    pubData.contextInfo = {1, 2, 3, 4, 5};
    pubData.header = block.toVbkEncoding();

    return pubData;
  }

  AltBlock generateNextBlock(const AltBlock& prev) {
    AltBlock block;
    block.hash = generateRandomBytesVector(32);
    block.height = prev.height + 1;
    block.previousBlock = prev.getHash();
    block.timestamp = prev.timestamp + 1;

    return block;
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer.mineBtcBlocks(1);
    return popminer.createVbkPopTxEndorsingVbkBlock(
        btcBlockTip->header,
        btctx,
        endorsedBlock,
        popminer.getBtcParams().getGenesisBlock().getHash());
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
