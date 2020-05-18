// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <util/test_utils.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/merkle_tree.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/mock_miner.hpp>

#include "util/fmtlogger.hpp"
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
    SetLogger<FmtLogger>();
    GetLogger().level = LogLevel::OFF;

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
      EXPECT_TRUE(alttree.setState(next.getHash(), state));
      index = alttree.getBlockIndex(next.getHash());
    }

    return const_cast<BlockIndex<AltBlock>*>(index);
  }

  void mineAltBlocks(uint32_t num, std::vector<AltBlock>& chain) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(alttree.acceptBlock(*chain.rbegin(), state));
      ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));
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

  VbkPopTx generatePopTx(const VbkBlock::hash_t& endorsedBlock) {
    auto index = popminer.vbk().getBlockIndex(endorsedBlock);
    if (!index) {
      throw std::logic_error("can't find endorsed block");
    }

    return generatePopTx(*index->header);
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto Btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer.mineBtcBlocks(1);
    return popminer.createVbkPopTxEndorsingVbkBlock(
        *btcBlockTip->header, Btctx, endorsedBlock, getLastKnownBtcBlock());
  }

  void fillVbkContext(VTB& vtb,
                      const VbkBlock::hash_t& lastKnownVbkBlockHash,
                      VbkBlockTree& tree) {
    fillVbkContext(vtb.context,
                   lastKnownVbkBlockHash,
                   vtb.containingBlock.getHash(),
                   tree);
  }

  void fillVbkContext(std::vector<VbkBlock>& out,
                      const VbkBlock::hash_t& lastKnownVbkBlockHash,
                      const VbkBlock::hash_t& containingBlock,
                      VbkBlockTree& tree) {
    auto* tip = tree.getBlockIndex(containingBlock);

    std::vector<VbkBlock> ctx;

    for (auto* walkBlock = tip;
         walkBlock != nullptr &&
         walkBlock->header->getHash() != lastKnownVbkBlockHash;
         walkBlock = walkBlock->pprev) {
      ctx.push_back(*walkBlock->header);
    }

    // since we inserted in reverse order, we need to reverse context blocks
    std::reverse(ctx.begin(), ctx.end());

    out.insert(out.end(), ctx.begin(), ctx.end());
  }

  PopData createPopData(int32_t version,
                        const ATV& atv,
                        std::vector<VTB> vtbs) {
    PopData popData;
    popData.version = version;

    // fill vbk context
    for (auto& vtb : vtbs) {
      for (const auto& block : vtb.context) {
        popData.vbk_context.push_back(block);
      }
      popData.vbk_context.push_back(vtb.containingBlock);
      vtb.context.clear();
    }

    for (const auto& block : atv.context) {
      popData.vbk_context.push_back(block);
    }

    popData.atv = atv;
    popData.atv.context.clear();
    popData.hasAtv = true;
    popData.vtbs = vtbs;

    return popData;
  }

  AltPayloads generateAltPayloads(const VbkTx& transaction,
                                  const AltBlock& containing,
                                  const AltBlock& endorsed,
                                  const VbkBlock::hash_t& lastVbk,
                                  int VTBs = 0) {
    PopData popData;

    for (auto i = 0; i < VTBs; i++) {
      auto vbkpoptx = generatePopTx(getLastKnownVbkBlock());
      auto vbkcontaining = popminer.applyVTB(popminer.vbk(), vbkpoptx, state);
      auto newvtb = popminer.vbkPayloads.at(vbkcontaining.getHash()).back();
      popData.vtbs.push_back(newvtb);
    }

    popData.hasAtv = true;
    popData.atv = popminer.generateATV(transaction, lastVbk, state);

    fillVbkContext(popData.vbk_context,
                   lastVbk,
                   popData.atv.containingBlock.getHash(),
                   popminer.vbk());

    AltPayloads alt;
    alt.popData = popData;
    alt.containingBlock = containing;
    alt.endorsed = endorsed;

    return alt;
  }

  AltPayloads endorseAltBlock(const AltBlock& endorsed,
                              const AltBlock& containing,
                              int VTBs = 0) {
    auto data = generatePublicationData(endorsed);
    auto vbktx = popminer.createVbkTxEndorsingAltBlock(data);
    return generateAltPayloads(
        vbktx, containing, endorsed, getLastKnownVbkBlock(), VTBs);
  }

  AltPayloads generateAltPayloads(const PopData& popTx,
                                  const AltBlock& containing,
                                  const AltBlock& endorsed) {
    AltPayloads alt;
    alt.popData = popTx;
    alt.containingBlock = containing;
    alt.endorsed = endorsed;

    return alt;
  }

  VbkBlock::hash_t getLastKnownVbkBlock() {
    return alttree.vbk().getBestChain().tip()->getHash();
  }

  BtcBlock::hash_t getLastKnownBtcBlock() {
    return alttree.btc().getBestChain().tip()->getHash();
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
