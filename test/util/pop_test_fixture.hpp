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
#include <veriblock/storage/inmem_block_storage.hpp>
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
  InmemPayloadsProvider payloadsProvider;
  InmemBlockStorage blockStorage;

  // miners
  std::shared_ptr<MockMiner> popminer;

  // trees
  AltBlockTree alttree =
      AltBlockTree(altparam, vbkparam, btcparam, payloadsProvider);

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

  void ConnectBlocksUntil(AltBlockTree& tree, const AltBlock::hash_t& hash) {
    auto* index = tree.getBlockIndex(hash);
    VBK_ASSERT(index);
    while (index && !index->hasFlags(BLOCK_CONNECTED)) {
      index->setFlag(BLOCK_CONNECTED);
      index->setFlag(BLOCK_HAS_PAYLOADS);
      index = index->pprev;
    }
  }

  bool SetState(AltBlockTree& tree,
                const AltBlock::hash_t& hash,
                bool connectBlocks = true) {
    if (connectBlocks) {
      ConnectBlocksUntil(tree, hash);
    }
    return tree.setState(hash, state);
  }

  bool AddPayloads(const AltBlock::hash_t& hash, const PopData& pop) {
    return AddPayloads(alttree, hash, pop);
  }

  bool AddPayloads(AltBlockTree& tree,
                   const AltBlock::hash_t& hash,
                   const PopData& pop) {
    popminer->getPayloadsProvider().write(pop);
    payloadsProvider.write(pop);

    auto index = tree.getBlockIndex(hash);
    EXPECT_TRUE(index);

    if(index->pprev) {
      ConnectBlocksUntil(tree, index->pprev->getHash());
    }
    return tree.addPayloads(hash, pop, state);
  }

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const PopData& popData) {
    auto* index = alttree.getBlockIndex(block_hash);
    if (!index) {
      return state.Invalid("bad-block", "Can't find containing block");
    }

    if (!AddPayloads(block_hash, popData)) {
      return state.Invalid("addPayloadsTemporarily");
    }

    if (!SetState(alttree, index->getHash())) {
      EXPECT_NO_FATAL_FAILURE(alttree.removePayloads(block_hash));
      return state.Invalid("addPayloadsTemporarily");
    }

    return true;
  }

  BlockIndex<AltBlock>* mineAltBlocks(const BlockIndex<AltBlock>& prev,
                                      size_t num,
                                      bool connectBlocks = false,
                                      bool setState = true) {
    const BlockIndex<AltBlock>* index = &prev;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(index->getHeader());
      EXPECT_TRUE(alttree.acceptBlockHeader(next, state));
      if (connectBlocks) {
        alttree.acceptBlock(next.getHash(), {});
      }
      if (setState) {
        EXPECT_TRUE(SetState(alttree, next.getHash()));
      }
      index = alttree.getBlockIndex(next.getHash());
    }

    return const_cast<BlockIndex<AltBlock>*>(index);
  }

  void mineAltBlocks(uint32_t num,
                     std::vector<AltBlock>& chain,
                     bool connectBlocks = false,
                     bool setState = true) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(alttree.acceptBlockHeader(*chain.rbegin(), state));
      if (connectBlocks) {
        alttree.acceptBlock(chain.back().getHash(), {});
      }
      if (setState) {
        ASSERT_TRUE(SetState(alttree, chain.rbegin()->getHash()));
      }
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
      EXPECT_TRUE(AddPayloads(nextBlock.getHash(), popdata));
      auto* next = alttree.getBlockIndex(nextBlock.getHash());
      VBK_ASSERT(next);
      EXPECT_TRUE(SetState(alttree, next->getHash()));
      altTip = next;
    }
  }

  template <typename index_t>
  std::vector<index_t> LoadBlocksFromDisk() {
    return blockStorage.load<typename index_t::block_t>();
  }

  template <typename index_t>
  typename index_t::hash_t LoadTipFromDisk() {
    return blockStorage.getTip<typename index_t::block_t>();
  }

  template <typename Tree>
  bool LoadTreeWrapper(Tree& tree) {
    using index_t = typename Tree::index_t;
    auto blocks = LoadBlocksFromDisk<index_t>();
    auto tip = LoadTipFromDisk<index_t>();
    return LoadTree<Tree>(tree, blocks, tip, state);
  }
};

namespace {

template <typename pop_t>
void validatePayloadsIndexState(PayloadsIndex& storage,
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
bool allPayloadsIsValid(PayloadsIndex& storage,
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

inline void validateAlttreeIndexState(AltBlockTree& tree,
                                      const AltBlock& containing,
                                      const PopData& popData,
                                      bool payloads_validation = true,
                                      bool payloads_existance = true) {
  auto& payloadsIndex = tree.getPayloadsIndex();
  auto& payloadsProvider = tree.getPayloadsProvider();
  auto containingHash = containing.getHash();

  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.context, payloads_existance);
  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.atvs, payloads_existance);
  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.vtbs, payloads_existance);

  std::vector<CommandGroup> commands;
  ValidationState state;
  ASSERT_TRUE(payloadsProvider.getCommands(
      tree, *tree.getBlockIndex(containingHash), commands, state))
      << state.toString();

  EXPECT_EQ(commands.size() == popData.context.size() + popData.atvs.size() +
                                   popData.vtbs.size(),
            payloads_existance);

  EXPECT_EQ(
      allPayloadsIsValid(payloadsIndex, containingHash, popData.context) &&
          allPayloadsIsValid(payloadsIndex, containingHash, popData.atvs) &&
          allPayloadsIsValid(payloadsIndex, containingHash, popData.vtbs),
      payloads_validation);
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
