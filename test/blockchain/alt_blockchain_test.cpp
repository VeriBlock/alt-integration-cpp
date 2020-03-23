#include <gtest/gtest.h>

#include <algorithm>

#include "util/test_utils.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/miner.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/storage/repository_rocks_manager.hpp"

using namespace altintegration;

// DB name
static const std::string dbName = "db-test";

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

auto mgr = std::make_shared<StateManager<RepositoryRocksManager>>(dbName);

struct AltTreeTest : public AltTree {
  AltTreeTest(const AltChainParams& config,
              const BtcChainParams& btcChainParams,
              const VbkChainParams& vbkChainParams)
      : AltTree(config,
                PopManager(config,
                           btcChainParams,
                           vbkChainParams,
                           mgr->getManager().getBtcEndorsementRepo(),
                           mgr->getManager().getVbkEndorsementRepo()),
                mgr->getManager().getPayloadsRepo()) {}
};

struct AltTreeTestSuite : public testing::Test {
  AltChainParamsTest config;
  BtcChainParamsRegTest btcChainParams;
  VbkChainParamsRegTest vbkChainParams;

  AltTreeTest altTree;

  MockMiner apm;

  ValidationState state;

  AltTreeTestSuite()
      : config(AltChainParamsTest()),
        btcChainParams(BtcChainParamsRegTest()),
        vbkChainParams(VbkChainParamsRegTest()),
        altTree(config, btcChainParams, vbkChainParams) {
    altTree.bootstrapWithGenesis(state);
    apm.btc().bootstrapWithGenesis(state);
    apm.vbk().bootstrapWithGenesis(state);
  }

  VbkBlock getLastKnownVbkBlock() {
    return altTree.currentPopManager().vbk().getBestChain().tip()->header;
  }

  BtcBlock getLastKnownBtcBlock() {
    return altTree.currentPopManager().btc().getBestChain().tip()->header;
  }

  std::vector<uint8_t> payoutInfo{1, 2, 3, 4, 5};

  AltBlock generateNextBlock(const AltBlock& last) {
    AltBlock next;
    next.previousBlock = last.hash;
    next.height = last.height + 1;
    next.hash = generateRandomBytesVector(32);
    next.timestamp = last.timestamp + 1;
    return next;
  }

  void mineAltChainFromBlock(std::vector<AltBlock>& blocks,
                             int size,
                             const AltBlock& fromBlock) {
    AltBlock newBlock = generateNextBlock(fromBlock);

    for (int i = 0; i < size; ++i) {
      EXPECT_TRUE(altTree.acceptBlock(newBlock, nullptr, state));
      EXPECT_TRUE(state.IsValid());
      blocks.push_back(newBlock);
      newBlock = generateNextBlock(newBlock);
    }
  }

  PublicationData endorseBlock(const std::vector<uint8_t>& block) {
    PublicationData pub;
    pub.header = block;
    pub.identifier = 1;
    pub.contextInfo = {};
    pub.payoutInfo = payoutInfo;
    return pub;
  }

  Payloads getPayloads(const AltBlock& endorsed, const AltBlock& prevBlock) {
    Publications pub = apm.mine(endorseBlock(endorsed.toVbkEncoding()),
                                getLastKnownVbkBlock().getHash(),
                                getLastKnownBtcBlock().getHash(),
                                0,
                                state);

    EXPECT_TRUE(state.IsValid());

    AltBlock containing = generateNextBlock(prevBlock);

    Payloads payloads;
    payloads.alt.containing = containing;
    payloads.alt.endorsed = endorsed;
    payloads.alt.atv = pub.atv;
    payloads.alt.hasAtv = true;
    payloads.vtbs = pub.vtbs;

    return payloads;
  }
};

TEST_F(AltTreeTestSuite, setState_test) {
  srand(0);

  std::vector<AltBlock> chain1;
  mineAltChainFromBlock(chain1, 1, config.getBootstrapBlock());

  EXPECT_TRUE(apm.mineBtcBlocks(14, state));
  EXPECT_TRUE(apm.mineVbkBlocks(30, state));

  Payloads payloads1 = getPayloads(*chain1.rbegin(), *chain1.rbegin());

  EXPECT_TRUE(altTree.acceptBlock(payloads1.alt.containing, &payloads1, state));

  mineAltChainFromBlock(chain1, 10, config.getBootstrapBlock());

  const BlockIndex<BtcBlock>* chain1StateBtcTip =
      apm.btc().getBestChain().tip()->pprev;
  const BlockIndex<VbkBlock>* chain1StateVbkTip =
      apm.vbk().getBestChain().tip()->pprev;

  EXPECT_EQ(altTree.currentPopManager().btc().getBestChain().tip()->getHash(),
            chain1StateBtcTip->getHash());

  EXPECT_EQ(altTree.currentPopManager().vbk().getBestChain().tip()->getHash(),
            chain1StateVbkTip->getHash());

  std::vector<AltBlock> chain2;
  mineAltChainFromBlock(chain2, 1, config.getBootstrapBlock());

  EXPECT_TRUE(apm.mineBtcBlocks(14, state));
  EXPECT_TRUE(apm.mineVbkBlocks(30, state));

  Payloads payloads2 = getPayloads(*chain2.rbegin(), *chain2.rbegin());

  EXPECT_TRUE(altTree.acceptBlock(payloads2.alt.containing, &payloads2, state));

  mineAltChainFromBlock(chain2, 15, config.getBootstrapBlock());

  const BlockIndex<BtcBlock>* chain2StateBtcTip =
      apm.btc().getBestChain().tip()->pprev;
  const BlockIndex<VbkBlock>* chain2StateVbkTip =
      apm.vbk().getBestChain().tip()->pprev;

  EXPECT_EQ(altTree.currentPopManager().btc().getBestChain().tip()->getHash(),
            chain2StateBtcTip->getHash());

  EXPECT_EQ(altTree.currentPopManager().vbk().getBestChain().tip()->getHash(),
            chain2StateVbkTip->getHash());

  for (size_t i = 1; i < std::max(chain1.size(), chain2.size()); ++i) {
    if (i < chain1.size()) {
      altTree.setState(chain1[i].getHash(), state);
      EXPECT_TRUE(state.IsValid());

      EXPECT_EQ(
          altTree.currentPopManager().btc().getBestChain().tip()->getHash(),
          chain1StateBtcTip->getHash());

      EXPECT_EQ(
          altTree.currentPopManager().vbk().getBestChain().tip()->getHash(),
          chain1StateVbkTip->getHash());
    }

    if (i < chain2.size()) {
      altTree.setState(chain2[i].getHash(), state);
      EXPECT_TRUE(state.IsValid());

      EXPECT_EQ(
          altTree.currentPopManager().btc().getBestChain().tip()->getHash(),
          chain2StateBtcTip->getHash());

      EXPECT_EQ(
          altTree.currentPopManager().vbk().getBestChain().tip()->getHash(),
          chain2StateVbkTip->getHash());
    }
  }
}
