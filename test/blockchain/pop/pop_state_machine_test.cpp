#include <gtest/gtest.h>

#include <vector>

#include "util/test_utils.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/blocktree.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/pop/pop_state_machine.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/payloads_repository_inmem.hpp"

using namespace altintegration;

template <typename BlockType>
void mineChain(std::vector<std::shared_ptr<BlockIndex<BlockType>>>& chain,
               int size,
               BlockIndex<BlockType>* fromBlock) {
  std::shared_ptr<BlockIndex<BlockType>> newBlock =
      std::make_shared<BlockIndex<BlockType>>(
          generateNextBlock<BlockType>(fromBlock));

  for (int i = 0; i < size; ++i) {
    chain.push_back(newBlock);
    newBlock = std::make_shared<BlockIndex<BlockType>>(
        generateNextBlock<BlockType>(newBlock.get()));
  }
}

PublicationData endorseBlock(const std::vector<uint8_t>& block) {
  static std::vector<uint8_t> payoutInfo = generateRandomBytesVector(32);

  PublicationData pub;
  pub.header = block;
  pub.identifier = 1;
  pub.contextInfo = {};
  pub.payoutInfo = payoutInfo;
  return pub;
}

VTB generatePayloads(MockMiner& apm,
                     const VbkBlock& endorsed,
                     const BtcBlock& lastKnownBlock,
                     ValidationState& state) {
  VTB pub = apm.generateAndApplyVTB(
      apm.vbk(), endorsed, lastKnownBlock.getHash(), state);

  EXPECT_TRUE(state.IsValid());

  return pub;
}

TEST(PopStateMachine, unapplyAndApply_test) {
  srand(0);

  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  ValidationState state;

  PayloadsRepositoryInmem<VTB> payloadsRepo;
  BtcChainParamsRegTest btcParams;
  VbkChainParamsRegTest vbkParams;
  BtcTree btcTree(btcParams);

  EXPECT_TRUE(btcTree.bootstrapWithGenesis(state));

  MockMiner apm;

  EXPECT_TRUE(apm.btc().bootstrapWithGenesis(state));
  EXPECT_TRUE(apm.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(apm.vbk().btc().bootstrapWithGenesis(state));

  std::shared_ptr<BlockIndex<VbkBlock>> bootstrapVbkBlock =
      std::make_shared<BlockIndex<VbkBlock>>(
          generateNextBlock<VbkBlock>(nullptr));

  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain1 = {
      bootstrapVbkBlock};

  std::vector<std::shared_ptr<BlockIndex<VbkBlock>>> chain2 = {
      bootstrapVbkBlock};

  mineChain(chain1, 1, bootstrapVbkBlock.get());
  mineChain(chain2, 1, bootstrapVbkBlock.get());

  PopStateMachine<BtcTree, BlockIndex<VbkBlock>, VbkChainParams> stateMachine(
      btcTree, bootstrapVbkBlock.get(), vbkParams, payloadsRepo);

  EXPECT_TRUE(apm.mineBtcBlocks(50, state));

  VTB payloads1 = generatePayloads(apm,
                                   chain1[chain1.size() - 1]->header,
                                   btcParams.getGenesisBlock(),
                                   state);

  payloadsRepo.put(payloads1);
  chain1[chain1.size() - 1]->containingPayloads.push_back(payloads1.getId());

  mineChain(chain1, 20, chain1[chain1.size() - 1].get());

  EXPECT_TRUE(stateMachine.unapplyAndApply(*chain1[chain1.size() - 1], state));

  const BlockIndex<BtcBlock>* chain1StateTip =
      apm.btc().getBestChain().tip()->pprev;

  EXPECT_EQ(chain1StateTip->getHash(), btcTree.getBestChain().tip()->getHash());

  EXPECT_TRUE(apm.mineBtcBlocks(50, state));

  VTB payloads2 = generatePayloads(apm,
                                   chain2[chain2.size() - 1]->header,
                                   btcParams.getGenesisBlock(),
                                   state);

  payloadsRepo.put(payloads2);
  chain2[chain2.size() - 1]->containingPayloads.push_back(payloads2.getId());

  mineChain(chain2, 20, chain2[chain2.size() - 1].get());

  EXPECT_TRUE(stateMachine.unapplyAndApply(*chain2[chain2.size() - 1], state));

  const BlockIndex<BtcBlock>* chain2StateTip =
      apm.btc().getBestChain().tip()->pprev;

  EXPECT_EQ(chain2StateTip->getHash(), btcTree.getBestChain().tip()->getHash());
  mineChain(chain2, 20, chain2[chain2.size() - 1].get());

  for (size_t i = 1; i < std::max(chain1.size(), chain2.size()); ++i) {
    if (i < chain1.size()) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*chain1[i], state));

      EXPECT_EQ(chain1StateTip->getHash(),
                btcTree.getBestChain().tip()->getHash());
    }

    if (i < chain2.size()) {
      EXPECT_TRUE(stateMachine.unapplyAndApply(*chain2[i], state));

      EXPECT_EQ(chain2StateTip->getHash(),
                btcTree.getBestChain().tip()->getHash());
    }
  }
}
