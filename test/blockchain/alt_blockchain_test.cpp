#include <gtest/gtest.h>

#include <algorithm>

#include "util/test_utils.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/mock_miner.hpp"

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

struct AltTreeTest : public testing::Test {
  ValidationState state;

  AltChainParamsTest altconfig{};
  VbkChainParamsRegTest vbkconfig{};
  BtcChainParamsRegTest btcconfig{};
  AltTree altTree;

  MockMiner popminer;

  AltTreeTest() : altTree(altconfig, vbkconfig, btcconfig) {
    altTree.bootstrapWithGenesis(state);
    altTree.vbk().bootstrapWithGenesis(state);
    altTree.vbk().btc().bootstrapWithGenesis(state);
  }

  void mineAltBlocks(uint32_t num, std::vector<AltBlock>& chain) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(altTree.acceptBlock(*chain.rbegin(), {}, state));
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
        btcBlockTip->header,
        Btctx,
        endorsedBlock,
        popminer.getBtcParams().getGenesisBlock().getHash());
  }

  void fillVTBContext(VTB& vtb,
                      const VbkBlock::hash_t& lastKnownVbkBlockHash,
                      VbkBlockTree& tree) {
    auto* tip = tree.getBlockIndex(vtb.containingBlock.getHash())->pprev;

    for (auto* walkBlock = tip;
         walkBlock->header.getHash() != lastKnownVbkBlockHash;
         walkBlock = walkBlock->pprev) {
      vtb.context.push_back(walkBlock->header);
    }

    // since we inserted in reverse order, we need to reverse context blocks
    std::reverse(vtb.context.begin(), vtb.context.end());
  }
};

TEST_F(AltTreeTest, acceptBlock_test) {
  std::vector<AltBlock> chain = {altconfig.getBootstrapBlock()};

  // mine 10 blocks
  mineAltBlocks(10, chain);

  AltBlock endorsedBlock = chain[5];

  VbkTx tx = popminer.endorseAltBlock(generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads1 =
      popminer.generateAltPayloads(tx,
                                   containingBlock,
                                   endorsedBlock,
                                   vbkconfig.getGenesisBlock().getHash(),
                                   state);

  // Generate VTB for 1 altPayloads1

  // mine 65 VBK blocks
  auto* vbkTip = popminer.mineVbkBlocks(65);

  // endorse VBK blocks
  auto* endorsedVbkBlock1 = vbkTip->getAncestor(vbkTip->height - 10);
  auto* endorsedVbkBlock2 = vbkTip->getAncestor(vbkTip->height - 11);
  generatePopTx(endorsedVbkBlock1->header);
  auto* btcBlockTip1 = popminer.btc().getBestChain().tip();
  popminer.mineBtcBlocks(100);
  generatePopTx(endorsedVbkBlock2->header);
  auto* btcBlockTip2 = popminer.btc().getBestChain().tip();

  vbkTip = popminer.mineVbkBlocks(1);

  auto vtbs = popminer.vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 2);
  fillVTBContext(
      vtbs[0], vbkconfig.getGenesisBlock().getHash(), popminer.vbk());
  fillVTBContext(
      vtbs[1], vbkconfig.getGenesisBlock().getHash(), popminer.vbk());

  // store vtbs in different altPayloads
  altPayloads1.vtbs = {vtbs[0]};

  EXPECT_TRUE(altTree.acceptBlock(
      containingBlock, {AltContext::fromContainer(altPayloads1)}, state));
  EXPECT_TRUE(state.IsValid());
  auto* containinVbkBlock = altTree.vbk().getBlockIndex(vbkTip->getHash());
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[0]).id) !=
              containinVbkBlock->containingEndorsements.end());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[1]).id) ==
              containinVbkBlock->containingEndorsements.end());

  // check btc tree state
  EXPECT_EQ(altTree.vbk().btc().getBestChain().tip()->getHash(),
            btcBlockTip1->getHash());

  mineAltBlocks(10, chain);
  containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);
  AltPayloads altPayloads2 =
      popminer.generateAltPayloads(tx,
                                   containingBlock,
                                   endorsedBlock,
                                   vbkconfig.getGenesisBlock().getHash(),
                                   state);

  altPayloads2.vtbs = {vtbs[1]};

  EXPECT_TRUE(altTree.acceptBlock(
      containingBlock, {AltContext::fromContainer(altPayloads2)}, state));
  EXPECT_TRUE(state.IsValid());

  containinVbkBlock = altTree.vbk().getBlockIndex(vbkTip->getHash());
  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[0]).id) !=
              containinVbkBlock->containingEndorsements.end());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[1]).id) !=
              containinVbkBlock->containingEndorsements.end());

  // check btc tree state
  EXPECT_EQ(altTree.vbk().btc().getBestChain().tip()->getHash(),
            btcBlockTip2->getHash());

  // reset state of the cmp_ in the altTree
  // generate new fork with the new altPayloads
  auto chain2 = chain;
  chain2.resize(chain.size() - 5);
  containingBlock = generateNextBlock(*chain2.rbegin());
  chain2.push_back(containingBlock);
  AltPayloads altPayloads3 =
      popminer.generateAltPayloads(tx,
                                   containingBlock,
                                   endorsedBlock,
                                   vbkconfig.getGenesisBlock().getHash(),
                                   state);

  EXPECT_TRUE(altTree.acceptBlock(
      containingBlock, {AltContext::fromContainer(altPayloads3)}, state));
  EXPECT_TRUE(state.IsValid());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[0]).id) !=
              containinVbkBlock->containingEndorsements.end());

  EXPECT_TRUE(containinVbkBlock->containingEndorsements.find(
                  BtcEndorsement::fromContainer(vtbs[1]).id) ==
              containinVbkBlock->containingEndorsements.end());

  // check btc tree state
  EXPECT_EQ(altTree.vbk().btc().getBestChain().tip()->getHash(),
            btcBlockTip1->getHash());
}
