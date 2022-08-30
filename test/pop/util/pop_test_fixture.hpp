// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <gtest/gtest.h>

#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/btc_chain_params.hpp>
#include <veriblock/pop/blockchain/vbk_chain_params.hpp>
#include <veriblock/pop/config.hpp>
#include <veriblock/pop/ct_params.hpp>
#include <veriblock/pop/entities/merkle_tree.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/mempool.hpp>
#include <veriblock/pop/mock_miner.hpp>
#include <veriblock/pop/storage/adaptors/block_provider_impl.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>
#include <veriblock/pop/storage/adaptors/payloads_provider_impl.hpp>
#include <veriblock/pop/storage/util.hpp>

#include "pop/util/comparator_test.hpp"
#include "pop/util/fmtlogger.hpp"
#include "pop/util/test_utils.hpp"

namespace altintegration {

struct AltTreeUnderTest : public AltBlockTree {
  size_t deallocatedAlt = 0;

  using AltBlockTree::AltBlockTree;
  void finalizeBlock(index_t& index) {
    this->finalizeBlockImpl(index, getParams().preserveBlocksBehindFinal());
  }
};

struct PopTestFixture {
  TestComparator cmp;

  const static std::vector<uint8_t> getPayoutInfo() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  }

  AltChainParamsRegTest altparam{};
  VbkChainParamsRegTest vbkparam{};
  BtcChainParamsRegTest btcparam{};
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl payloadsProvider{storage};
  adaptors::BlockReaderImpl blockProvider{storage, altparam};

  // miners
  MockMiner popminer{altparam, vbkparam, btcparam};

  // trees
  AltTreeUnderTest alttree{
      altparam, vbkparam, btcparam, payloadsProvider, blockProvider};

  MemPool mempool{alttree};

  ValidationState state;

  PopTestFixture() {
    auto BTCgenesis = GetRegTestBtcBlock();
    auto VBKgenesis = GetRegTestVbkBlock();

    // by default, set mocktime to the latest time between all genesis blocks
    auto time = std::max({altparam.getBootstrapBlock().getTimestamp(),
                          BTCgenesis.getTimestamp(),
                          VBKgenesis.getTimestamp()});
    setMockTime(time + 1);

    EXPECT_NO_FATAL_FAILURE(alttree.btc().bootstrapWithGenesis(BTCgenesis));
    EXPECT_NO_FATAL_FAILURE(alttree.vbk().bootstrapWithGenesis(VBKgenesis));
    EXPECT_NO_FATAL_FAILURE(alttree.bootstrap());
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
    auto& bop = blockOfProof->getBlockOfProofEndorsement();

    auto _ = [&](const E* end) -> bool { return end->id == e.id; };
    EXPECT_EQ(std::count_if(bop.begin(), bop.end(), _), 1);
    auto* endorsed = tree.getBlockIndex(e.endorsedHash);
    ASSERT_TRUE(endorsed) << "no endorsed block " << HexStr(e.endorsedHash);
    const auto& by = endorsed->getEndorsedBy();
    EXPECT_EQ(std::count_if(by.begin(), by.end(), _), 1);
  }

  void ConnectBlocksUntil(AltBlockTree& tree, const AltBlock::hash_t& hash) {
    auto* index = tree.getBlockIndex(hash);
    VBK_ASSERT(index);
    std::vector<BlockIndex<AltBlock>*> indices;
    while (index != nullptr && !index->isValidUpTo(BLOCK_CONNECTED)) {
      indices.push_back(index);
      index = index->pprev;
    }
    std::reverse(indices.begin(), indices.end());
    for (auto* i : indices) {
      i->setFlag(BLOCK_HAS_PAYLOADS);
      VBK_ASSERT(i->raiseValidity(BLOCK_CONNECTED));
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
    auto index = tree.getBlockIndex(hash);
    EXPECT_TRUE(index);

    VBK_ASSERT(index != nullptr);
    if (index->pprev != nullptr) {
      ConnectBlocksUntil(tree, index->pprev->getHash());
    }

    tree.acceptBlock(*index, pop, state);
    return state.IsValid();
  }

  bool validatePayloads(const AltBlock::hash_t& block_hash,
                        const PopData& popData) {
    auto* index = alttree.getBlockIndex(block_hash);
    if (index == nullptr) {
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
                                      bool connectBlocks = true,
                                      bool setState = true) {
    const BlockIndex<AltBlock>* index = &prev;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(index->getHeader());
      EXPECT_TRUE(alttree.acceptBlockHeader(next, state)) << state.toString();
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

  BlockIndex<AltBlock>* mineAltBlocks(const BlockIndex<AltBlock>& prev,
                                      size_t num,
                                      const PopData& pd,
                                      bool setState = true) {
    const BlockIndex<AltBlock>* index = &prev;
    // first block will contain given PopData, others will not.
    auto pop = pd;
    for (size_t i = 0; i < num; i++) {
      auto next = generateNextBlock(index->getHeader());
      EXPECT_TRUE(alttree.acceptBlockHeader(next, state)) << state.toString();
      alttree.acceptBlock(next.getHash(), pop);
      pop = {};
      if (setState) {
        EXPECT_TRUE(SetState(alttree, next.getHash()));
      }
      index = alttree.getBlockIndex(next.getHash());
    }

    return const_cast<BlockIndex<AltBlock>*>(index);
  }

  void mineAltBlocks(uint32_t num,
                     std::vector<AltBlock>& chain,
                     bool connectBlocks = true,
                     bool setState = true) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(chain.back()));

      ASSERT_TRUE(alttree.acceptBlockHeader(chain.back(), state))
          << state.toString();
      if (connectBlocks) {
        alttree.acceptBlock(chain.back().getHash(), {});
      }
      if (setState) {
        ASSERT_TRUE(SetState(alttree, chain.back().getHash()));
      }
      ASSERT_TRUE(state.IsValid()) << state.toString();
    }
  }

  PublicationData generatePublicationData(const AltBlock& endorsed,
                                          uint256 stateRoot = uint256()) {
    return generatePublicationData(alttree, endorsed, stateRoot);
  }

  PublicationData generatePublicationData(AltBlockTree& tree,
                                          const AltBlock& endorsed,
                                          uint256 stateRoot = uint256()) {
    PublicationData pubData;
    pubData.payoutInfo = getPayoutInfo();
    pubData.identifier = altparam.getIdentifier();
    pubData.header = endorsed.toRaw();
    VBK_ASSERT(endorsed.hash.size() == ALT_HASH_SIZE);
    VBK_ASSERT(endorsed.previousBlock.size() == ALT_HASH_SIZE);

    const auto* prev = tree.getBlockIndex(endorsed.previousBlock);
    auto c = AuthenticatedContextInfoContainer::createFromPrevious(
        stateRoot, prev, altparam);
    pubData.contextInfo = SerializeToVbkEncoding(c);

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
    if (index == nullptr) {
      throw std::logic_error("can't find endorsed block");
    }

    return generatePopTx(index->getHeader());
  }

  VbkPopTx generatePopTx(const VbkBlock& endorsedBlock) {
    auto btctx = popminer.createBtcTxEndorsingVbkBlock(endorsedBlock);
    auto* btcblock = popminer.mineBtcBlocks(1, {btctx});
    return popminer.createVbkPopTxEndorsingVbkBlock(
        btcblock->getHeader(), btctx, endorsedBlock, getLastKnownBtcBlock());
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
      auto vbkcontaining = popminer.mineVbkBlocks(1, {vbkpoptx});
      auto newvtb = popminer.createVTB(vbkcontaining->getHeader(), vbkpoptx);
      popData.vtbs.push_back(newvtb);
    }

    for (const auto& tx : transactions) {
      auto* block = popminer.mineVbkBlocks(1, {tx});
      ATV atv = popminer.createATV(block->getHeader(), tx);
      popData.atvs.push_back(atv);
    }

    fillVbkContext(popData.context, lastVbk, popminer.vbk());

    return popData;
  }

  PopData endorseAltBlock(const std::vector<AltBlock>& endorsed,
                          size_t VTBs = 0) {
    std::vector<VbkTx> transactions(endorsed.size());
    for (size_t i = 0; i < endorsed.size(); ++i) {
      auto data = generatePublicationData(endorsed[i]);
      transactions[i] = popminer.createVbkTxEndorsingAltBlock(data);
    }
    return generateAltPayloads(transactions, getLastKnownVbkBlock(), VTBs);
  }

  VbkBlock::hash_t getLastKnownVbkBlock() {
    return alttree.vbk().getBestChain().tip()->getHash();
  }

  BtcBlock::hash_t getLastKnownBtcBlock() {
    return alttree.btc().getBestChain().tip()->getHash();
  }

  VbkPopTx endorseVbkTip() {
    auto* tip = popminer.vbk().getBestChain().tip();
    VBK_ASSERT(tip);
    return popminer.createVbkPopTxEndorsingVbkBlock(tip->getHeader(),
                                                    getLastKnownBtcBlock());
  }

  void createEndorsedAltChain(size_t blocks, size_t vtbs = 1) {
    std::vector<VbkPopTx> transactions(vtbs);
    for (size_t i = 0; i < vtbs; i++) {
      transactions[i] = endorseVbkTip();
    }
    popminer.mineVbkBlocks(1, transactions);

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

  void save(AltBlockTree& tree) {
    auto batch = storage.generateWriteBatch();
    auto writer = adaptors::BlockBatchImpl(*batch);
    saveTrees(tree, writer);
    batch->writeBatch();
  }

  bool load(AltBlockTree& tree) { return loadTrees(tree, false, state); }
};

template <typename pop_t>
void validatePayloadsIndexState(
    const PayloadsIndex<BlockIndex<AltBlock>>& storage,
    const AltBlock::hash_t& containingHash,
    const std::vector<pop_t>& payloads,
    bool payloads_existance) {
  for (const auto& data : payloads) {
    auto alt_set = storage.find(data.getId().asVector());
    EXPECT_EQ(alt_set.find(containingHash) != alt_set.end(),
              payloads_existance);
  }
}

template <typename Tree>
void assertBlockTreeHasNoOrphans(const Tree& tree) {
  // orphan = block whose pprev == nullptr, and this block is not `genesis`
  // (root/bootstrap) block
  for (auto& index : tree.getBlocks()) {
    ASSERT_TRUE(index);

    if (index->pprev == nullptr &&
        index->getHash() != tree.getRoot().getHash()) {
      FAIL() << "block " << index->toPrettyString() << " is orphan!";
    }
  }
}

inline void assertTreesHaveNoOrphans(AltBlockTree& tree) {
  assertBlockTreeHasNoOrphans(tree);
  assertBlockTreeHasNoOrphans(tree.vbk());
  assertBlockTreeHasNoOrphans(tree.btc());
}

template <typename Tree>
void assertTreeTips(Tree& tree, std::vector<typename Tree::index_t*> expected) {
  const auto& tips = tree.getTips();
  EXPECT_EQ(tips.size(), expected.size());
  for (auto* i : expected) {
    EXPECT_TRUE(tips.count(i));
  }
}

inline void validateAlttreeIndexState(AltBlockTree& tree,
                                      const AltBlock& containing,
                                      const PopData& popData,
                                      bool payloads_existance = true) {
  const auto& payloadsIndex = tree.getPayloadsIndex();
  auto& commandGroupStore = tree.getCommandGroupStore();
  const auto& containingHash = containing.getHash();

  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.context, payloads_existance);
  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.atvs, payloads_existance);
  validatePayloadsIndexState(
      payloadsIndex, containingHash, popData.vtbs, payloads_existance);

  std::unique_ptr<AltCommandGroupStore::command_groups_t> commands;
  ValidationState state;
  EXPECT_NO_THROW(commands = commandGroupStore.getCommands(
                      *tree.getBlockIndex(containingHash), state))
      << state.toString();

  ASSERT_NE(commands, nullptr);
  EXPECT_EQ(commands->size() == popData.context.size() + popData.atvs.size() +
                                    popData.vtbs.size(),
            payloads_existance);
}

template <typename Payload, typename Tree>
std::set<typename Payload::id_t> getAllPayloadIdsInTree(
    Tree& tree, bool skipFinal = false) {
  std::set<typename Payload::id_t> ids;
  for (auto* index : tree.getAllBlocks()) {
    VBK_ASSERT(index != nullptr);
    if (skipFinal && index->finalized) {
      continue;
    }

    for (const auto& id : index->template getPayloadIds<Payload>()) {
      ids.insert(id);
    }
  }

  return ids;
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
