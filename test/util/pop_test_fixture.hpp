#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <util/test_utils.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/merkle_tree.hpp>
#include <veriblock/mock_miner.hpp>

#include "util/test_utils.hpp"

namespace altintegration {

struct PopTestFixture {
  const static std::vector<uint8_t> getPayoutInfo() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  }

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

  BlockIndex<AltBlock>* mineAltBlocks(const BlockIndex<AltBlock>& prev,
                                      size_t num) {
    const BlockIndex<AltBlock>* index = &prev;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(*index->header);
      EXPECT_TRUE(alttree.acceptBlock(next, state));
      index = alttree.getBlockIndex(next.getHash());
    }

    return const_cast<BlockIndex<AltBlock>*>(index);
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
    PublicationData pubData;
    pubData.payoutInfo = getPayoutInfo();
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
    auto Btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer.mineBtcBlocks(1);
    return popminer.createVbkPopTxEndorsingVbkBlock(
        *btcBlockTip->header,
        Btctx,
        endorsedBlock,
        popminer.getBtcParams().getGenesisBlock().getHash());
  }

  void fillVTBContext(VTB& vtb,
                      const VbkBlock::hash_t& lastKnownVbkBlockHash,
                      VbkBlockTree& tree) {
    auto* tip = tree.getBlockIndex(vtb.containingBlock.getHash())->pprev;

    for (auto* walkBlock = tip;
         walkBlock != nullptr &&
         walkBlock->header->getHash() != lastKnownVbkBlockHash;
         walkBlock = walkBlock->pprev) {
      vtb.context.push_back(*walkBlock->header);
    }

    // since we inserted in reverse order, we need to reverse context blocks
    std::reverse(vtb.context.begin(), vtb.context.end());
  }

  AltPayloads generateAltPayloads(
      const VbkTx& transaction,
      const AltBlock& containing,
      const AltBlock& endorsed,
      const VbkBlock::hash_t& lastKnownVbkBlockHash) {
    AltPopTx altPopTx;
    altPopTx.hasAtv = true;
    altPopTx.atv =
        popminer.generateATV(transaction, lastKnownVbkBlockHash, state);

    AltPayloads alt;
    alt.altPopTx = altPopTx;
    alt.containingBlock = containing;
    alt.endorsed = endorsed;
    return alt;
  }

  AltPayloads generateAltPayloads(const AltPopTx& popTx,
                                  const AltBlock& containing,
                                  const AltBlock& endorsed) {
    AltPayloads alt;
    alt.altPopTx = popTx;
    alt.containingBlock = containing;
    alt.endorsed = endorsed;

    return alt;
  }

  VbkBlock::hash_t getLastKnownVbkBlock() {
    return alttree.vbk().getBestChain().tip()->getHash();
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
