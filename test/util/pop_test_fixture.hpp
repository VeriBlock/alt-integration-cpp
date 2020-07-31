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
#include <veriblock/mempool.hpp>
#include <veriblock/mock_miner.hpp>
#include <veriblock/storage/inmem/storage_manager_inmem.hpp>
#include <veriblock/storage/util.hpp>

#include "util/fmtlogger.hpp"
#include "util/test_utils.hpp"
#include "util/comparator_test.hpp"

namespace altintegration {

struct PopTestFixture {
  const static std::vector<uint8_t> getPayoutInfo() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  }

  std::shared_ptr<MemPool> mempool;

  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};
  StorageManagerInmem storageManager{};
  PayloadsStorage& storagePayloads = storageManager.getPayloadsStorage();

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

    mempool = std::make_shared<MemPool>(alttree);
  }

  BlockIndex<AltBlock>* mineAltBlocks(const BlockIndex<AltBlock>& prev,
                                      size_t num) {
    const BlockIndex<AltBlock>* index = &prev;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(index->getHeader());
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

  PopData mergePopData(const std::vector<PopData>& all) {
    PopData out;
    for (auto& p : all) {
      out.mergeFrom(p);
    }
    return out;
  }

  VbkPopTx generatePopTx(const VbkBlock::hash_t& endorsedBlock) {
    auto index = popminer->vbk().getBlockIndex(endorsedBlock);
    if (!index) {
      throw std::logic_error("can't find endorsed block");
    }

    return generatePopTx(index->getHeader());
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto Btctx = popminer->createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcBlockTip = popminer->mineBtcBlocks(1);
    return popminer->createVbkPopTxEndorsingVbkBlock(
        btcBlockTip->getHeader(), Btctx, endorsedBlock, getLastKnownBtcBlock());
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
         walkBlock->getHeader().getHash() != lastKnownVbkBlockHash;
         walkBlock = walkBlock->pprev) {
      if (known_blocks.count(walkBlock->getHeader().getHash()) == 0) {
        ctx.push_back(walkBlock->getHeader());
      }
    }

    // since we inserted in reverse order, we need to reverse context blocks
    std::reverse(ctx.begin(), ctx.end());

    out.insert(out.end(), ctx.begin(), ctx.end());
  }

  PopData createPopData(std::vector<ATV> atvs, std::vector<VTB> vtbs) {
    PopData popData;

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

      if (known_blocks.count(atv.blockOfProof.getHash()) == 0) {
        popData.context.push_back(atv.blockOfProof);
        known_blocks.insert(atv.blockOfProof.getHash());
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
                     atv.blockOfProof.getHash(),
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

  void endorseVbkTip() {
    auto* tip = popminer->vbk().getBestChain().tip();
    VBK_ASSERT(tip);
    auto tx = popminer->endorseVbkBlock(
        tip->getHeader(), getLastKnownBtcBlock(), state);
    popminer->vbkmempool.push_back(tx);
  }

  void createEndorsedAltChain(size_t blocks, size_t vtbs = 1) {
    for (size_t i = 0; i < vtbs; i++) {
      endorseVbkTip();
    }
    popminer->mineVbkBlocks(1);

    auto* altTip = alttree.getBestChain().tip();
    VBK_ASSERT(altTip);
    for (size_t i = 0; i < blocks; i++) {
      auto nextBlock = generateNextBlock(altTip->getHeader());
      auto popdata = endorseAltBlock({altTip->getHeader()}, vtbs);
      EXPECT_TRUE(alttree.acceptBlock(nextBlock, state));
      EXPECT_TRUE(alttree.addPayloads(nextBlock, popdata, state));
      auto* next = alttree.getBlockIndex(nextBlock.getHash());
      VBK_ASSERT(next);
      EXPECT_TRUE(alttree.setState(*next, state));
      altTip = next;
    }
  }
};

template <typename index_t>
std::vector<index_t> LoadBlocksFromDisk(PopStorage& storage) {
  auto map = storage.loadBlocks<index_t>();
  std::vector<index_t> ret;
  for (auto& pair : map) {
    ret.push_back(*pair.second);
  }

  std::sort(ret.begin(), ret.end(), [](const index_t& a, const index_t& b) {
    return a.getHeight() < b.getHeight();
  });

  return ret;
}

template <typename index_t>
typename index_t::hash_t LoadTipFromDisk(PopStorage& storage) {
  auto tip = storage.loadTip<index_t>();
  return tip.second;
}

template <typename Tree>
bool LoadTreeWrapper(Tree& tree, PopStorage& storage, ValidationState& state) {
  using index_t = typename Tree::index_t;
  auto blocks = LoadBlocksFromDisk<index_t>(storage);
  auto tip = LoadTipFromDisk<index_t>(storage);
  return LoadTree<Tree>(tree, blocks, tip, state);
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
