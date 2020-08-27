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

#include "util/comparator_test.hpp"
#include "util/fmtlogger.hpp"
#include "util/test_utils.hpp"

namespace altintegration {

struct PopTestFixture {
  TestComparator cmp;

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
    // by default, set mocktime to the latest time between all genesis blocks
    auto time = std::max({altparam.getBootstrapBlock().getBlockTime(),
                          vbkparam.getGenesisBlock().getBlockTime(),
                          btcparam.getGenesisBlock().getBlockTime()});
    setMockTime(time + 1);

    EXPECT_TRUE(alttree.btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alttree.bootstrap(state));

    popminer = std::make_shared<MockMiner>();

    mempool = std::make_shared<MemPool>(alttree);
  }

  // T = tree
  // E = endorsement
  template <typename T, typename E>
  void verifyEndorsementAdded(T& tree, const E& e) {
    auto* containing = tree.getBlockIndex(e.containingHash);
    ASSERT_TRUE(containing)
        << "no containing block " << HexStr(e.containingHash);
    EXPECT_EQ(containing->getContainingEndorsements().count(e.id), 1);
    const auto& comparator = tree.getComparator();
    auto* blockOfProof =
        comparator.getProtectingBlockTree().getBlockIndex(e.blockOfProof);
    ASSERT_TRUE(blockOfProof) << "no blockOfProof " << HexStr(e.blockOfProof);
    auto& bop = blockOfProof->blockOfProofEndorsements;

    auto _ = [&](const E* end) -> bool { return end->id == e.id; };
    EXPECT_EQ(std::count_if(bop.begin(), bop.end(), _), 1);
    auto* endorsed = tree.getBlockIndex(e.endorsedHash);
    ASSERT_TRUE(endorsed) << "no endorsed block " << HexStr(e.endorsedHash);
    auto& by = endorsed->endorsedBy;
    EXPECT_EQ(std::count_if(by.begin(), by.end(), _), 1);
  }

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const PopData& popData,
                        ValidationState& _state) {
    auto* index = alttree.getBlockIndex(block_hash);
    if (!index) {
      return _state.Invalid("bad-block", "Can't find containing block");
    }

    if (!alttree.addPayloads(block_hash, popData, _state)) {
      return _state.Invalid("addPayloadsTemporarily");
    }

    if (!alttree.setState(*index, _state)) {
      EXPECT_NO_FATAL_FAILURE(alttree.removePayloads(block_hash));
      return _state.Invalid("addPayloadsTemporarily");
    }

    return true;
  }

  BlockIndex<AltBlock>* mineAltBlocks(const BlockIndex<AltBlock>& prev,
                                      size_t num) {
    const BlockIndex<AltBlock>* index = &prev;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(index->getHeader());
      EXPECT_TRUE(alttree.acceptBlockHeader(next, state));
      EXPECT_TRUE(alttree.setState(next.getHash(), state));
      index = alttree.getBlockIndex(next.getHash());
    }

    return const_cast<BlockIndex<AltBlock>*>(index);
  }

  void mineAltBlocks(uint32_t num, std::vector<AltBlock>& chain) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(alttree.acceptBlockHeader(*chain.rbegin(), state));
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

  void fillVbkContext(std::vector<VbkBlock>& out,
                      const VbkBlock::hash_t& lastKnownVbkBlockHash,
                      VbkBlockTree& tree) {
    fillVbkContext(
        out, lastKnownVbkBlockHash, tree.getBestChain().tip()->getHash(), tree);
  }

  PopData generateAltPayloads(const std::vector<VbkTx>& transactions,
                              const VbkBlock::hash_t& lastVbk,
                              size_t VTBs = 0) {
    PopData popData;

    for (size_t i = 0; i < VTBs; i++) {
      auto vbkpoptx = generatePopTx(getLastKnownVbkBlock());
      auto vbkcontaining = popminer->applyVTB(popminer->vbk(), vbkpoptx, state);
      auto newvtb = popminer->vbkPayloads.at(vbkcontaining.getHash()).back();
      popData.vtbs.push_back(newvtb);
    }

    for (const auto& t : transactions) {
      popData.atvs.push_back(popminer->applyATV(t, state));
    }

    fillVbkContext(popData.context, lastVbk, popminer->vbk());

    return popData;
  }

  PopData endorseAltBlock(const std::vector<AltBlock>& endorsed,
                          size_t VTBs = 0) {
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
      EXPECT_TRUE(alttree.acceptBlockHeader(nextBlock, state));
      EXPECT_TRUE(alttree.addPayloads(nextBlock.getHash(), popdata, state));
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

namespace {

template <typename pop_t>
void validatePayloadsIndexState(PayloadsStorage& storage,
                                const AltBlock::hash_t& containingHash,
                                const std::vector<pop_t>& payloads,
                                bool payloads_existance) {
  for (const auto& data : payloads) {
    auto alt_set = storage.getContainingAltBlocks(data.getId().asVector());
    EXPECT_EQ(alt_set.find(containingHash) != alt_set.end(),
              payloads_existance);
  }
}

template <typename pop_t>
bool allPayloadsIsValid(PayloadsStorage& storage,
                        const AltBlock::hash_t& containingHash,
                        const std::vector<pop_t>& payloads) {
  for (const auto& p : payloads) {
    auto id = p.getId();
    if (!storage.getValidity(containingHash, id)) {
      return false;
    }
  }

  return true;
}

}  // namespace

inline void validateAlttreeIndexState(AltTree& tree,
                                      const AltBlock& containing,
                                      const PopData& popData,
                                      bool payloads_validation = true,
                                      bool payloads_existance = true) {
  auto& storage = tree.getStorage();
  auto containingHash = containing.getHash();

  validatePayloadsIndexState(
      storage, containingHash, popData.context, payloads_existance);
  validatePayloadsIndexState(
      storage, containingHash, popData.atvs, payloads_existance);
  validatePayloadsIndexState(
      storage, containingHash, popData.vtbs, payloads_existance);

  std::vector<CommandGroup> commands =
      storage.loadCommands(*tree.getBlockIndex(containingHash), tree);

  EXPECT_EQ(commands.size() == popData.context.size() + popData.atvs.size() +
                                   popData.vtbs.size(),
            payloads_existance);

  EXPECT_EQ(allPayloadsIsValid(storage, containingHash, popData.context) &&
                allPayloadsIsValid(storage, containingHash, popData.atvs) &&
                allPayloadsIsValid(storage, containingHash, popData.vtbs),
            payloads_validation);
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
