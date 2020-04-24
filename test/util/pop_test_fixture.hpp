#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <util/test_utils.hpp>
#include <veriblock/alt-util.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/merkle_tree.hpp>
#include <veriblock/mock_miner.hpp>

#include "util/test_utils.hpp"

namespace altintegration {

struct PopTestFixture {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};

  // miners
  MockMiner popminer{};

  // trees
  AltTree alttree;

  ValidationState state;

  PopTestFixture() : alttree(makeEmptyAltTree()) {}

  AltTree makeEmptyAltTree() {
    auto t = AltTree(altparam, vbkparam, btcparam);
    EXPECT_TRUE(t.bootstrap(state));
    EXPECT_TRUE(t.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(t.vbk().btc().bootstrapWithGenesis(state));
    return t;
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
    AltPayloads alt;
    alt.hasAtv = true;
    alt.atv = popminer.generateATV(transaction, lastKnownVbkBlockHash, state);
    alt.containingBlock = containing;
    alt.endorsed = endorsed;
    return alt;
  }

  BtcBlock::hash_t getLastKnownBtcBlock() {
    auto* tip = alttree.btc().getBestChain().tip();
    EXPECT_TRUE(tip);
    return tip->getHash();
  }

  VbkBlock::hash_t getLastKnownVbkBlock() {
    auto* tip = alttree.vbk().getBestChain().tip();
    EXPECT_TRUE(tip);
    return tip->getHash();
  }

  VTB endorseVbkBlock(const BlockIndex<VbkBlock>& endorsed) {
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(*endorsed.header);
    auto* btcContaining = popminer.mineBtcBlocks(1);
    auto vbktx =
        popminer.createVbkPopTxEndorsingVbkBlock(*btcContaining->header,
                                                 btctx,
                                                 *endorsed.header,
                                                 getLastKnownBtcBlock());
    auto* vbkContaining = popminer.mineVbkBlocks(1);

    auto& vtbs = popminer.vbkPayloads[vbkContaining->getHash()];
    EXPECT_FALSE(vtbs.empty());

    auto& lastVtb = *vtbs.rbegin();
    fillVTBContext(lastVtb, getLastKnownVbkBlock(), alttree.vbk());

    return lastVtb;
  }

  VTB endorseVbkBlock(int height) {
    auto* endorsed = popminer.vbk().getBestChain()[height];
    if (!endorsed) {
      throw std::logic_error("can't find endorsed block at height " +
                             std::to_string(height));
    }

    return endorseVbkBlock(*endorsed);
  }

  AltPayloads endorseAltBlock(const AltBlock& endorsed,
                              const AltBlock& containing) {
    return endorseAltBlock(endorsed.hash, containing);
  }

  AltPayloads endorseAltBlock(const AltBlock::hash_t& endorsed,
                              const AltBlock& containing) {
    auto* endorsedIndex = alttree.getBlockIndex(endorsed);
    if (!endorsedIndex) {
      throw std::logic_error("can't find alt endorsed hash: " +
                             HexStr(endorsed));
    }

    return endorseAltBlock(*endorsedIndex, containing);
  }
  AltPayloads endorseAltBlock(const BlockIndex<AltBlock>& endorsed,
                              const AltBlock& containing) {
    auto pub = generatePublicationData(*endorsed.header);
    auto vbktx = popminer.endorseAltBlock(pub);
    return generateAltPayloads(
        vbktx, containing, *endorsed.header, getLastKnownVbkBlock());
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
