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
  PayloadsStorage storagePayloads{};

  // miners
  std::shared_ptr<MockMiner> popminer;

  // trees
  AltTree alttree = AltTree(altparam, vbkparam, btcparam, storagePayloads);

  ValidationState state;

  PopTestFixture() {
    SetLogger<FmtLogger>();
    GetLogger().level = LogLevel::off;

    EXPECT_TRUE(alttree.btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.bootstrap(state));

    popminer = std::make_shared<MockMiner>();
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
    auto index = popminer->vbk().getBlockIndex(endorsedBlock);
    if (!index) {
      throw std::logic_error("can't find endorsed block");
    }

    return generatePopTx(*index->header);
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto Btctx = popminer->createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer->mineBtcBlocks(1);
    return popminer->createVbkPopTxEndorsingVbkBlock(
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

    std::set<typename VbkBlock::hash_t> known_blocks;
    for (const auto& b : out) {
      known_blocks.insert(b.getHash());
    }

    for (auto* walkBlock = tip;
         walkBlock != nullptr &&
         walkBlock->header->getHash() != lastKnownVbkBlockHash;
         walkBlock = walkBlock->pprev) {
      if (known_blocks.count(walkBlock->header->getHash()) == 0) {
        ctx.push_back(*walkBlock->header);
      }
    }

    // since we inserted in reverse order, we need to reverse context blocks
    std::reverse(ctx.begin(), ctx.end());

    out.insert(out.end(), ctx.begin(), ctx.end());
  }

  PopData createPopData(int32_t version,
                        std::vector<ATV> atvs,
                        std::vector<VTB> vtbs) {
    PopData popData;
    popData.version = version;

    std::set<typename VbkBlock::hash_t> known_blocks;

    // fill vbk context
    for (auto& vtb : vtbs) {
      for (const auto& block : vtb.context) {
        if (known_blocks.count(block.getHash()) == 0) {
          popData.context.push_back(block);
          known_blocks.insert(block.getHash());
        }
      }

      if (known_blocks.count(vtb.containingBlock.getHash()) == 0) {
        popData.context.push_back(vtb.containingBlock);
        known_blocks.insert(vtb.containingBlock.getHash());
      }

      vtb.context.clear();
    }

    for (auto& atv : atvs) {
      for (const auto& block : atv.context) {
        if (known_blocks.count(block.getHash()) == 0) {
          popData.context.push_back(block);
          known_blocks.insert(block.getHash());
        }
      }

      if (known_blocks.count(atv.containingBlock.getHash()) == 0) {
          popData.context.push_back(atv.containingBlock);
          known_blocks.insert(atv.containingBlock.getHash());
      }

      atv.context.clear();
    }

    std::sort(popData.context.begin(),
              popData.context.end(),
              [](const VbkBlock& a, const VbkBlock& b) {
                return a.height < b.height;
              });

    popData.atvs = atvs;
    popData.vtbs = vtbs;

    return popData;
  }

  PopData generateAltPayloads(const std::vector<VbkTx>& transactions,
                              const VbkBlock::hash_t& lastVbk,
                              int VTBs = 0) {
    PopData popData;

    for (auto i = 0; i < VTBs; i++) {
      auto vbkpoptx = generatePopTx(getLastKnownVbkBlock());
      auto vbkcontaining = popminer->applyVTB(popminer->vbk(), vbkpoptx, state);
      auto newvtb = popminer->vbkPayloads.at(vbkcontaining.getHash()).back();
      popData.vtbs.push_back(newvtb);
    }

    for (const auto& t : transactions) {
      popData.atvs.push_back(popminer->generateATV(t, lastVbk, state));
    }

    for (const auto& atv : popData.atvs) {
      fillVbkContext(popData.context,
                     lastVbk,
                     atv.containingBlock.getHash(),
                     popminer->vbk());
    }

    return popData;
  }

  PopData endorseAltBlock(const std::vector<AltBlock>& endorsed, int VTBs = 0) {
    std::vector<VbkTx> transactions(endorsed.size());
    for (size_t i = 0; i < endorsed.size(); ++i) {
      auto data = generatePublicationData(endorsed[i]);
      transactions[i] = popminer->createVbkTxEndorsingAltBlock(data);
    }
    return generateAltPayloads(transactions, getLastKnownVbkBlock(), VTBs);
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
