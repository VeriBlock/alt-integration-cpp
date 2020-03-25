#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <veriblock/mock_miner.hpp>
#include <veriblock/popmanager.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository_inmem.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>
#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/rewards/poprewards.hpp"

using namespace altintegration;

static const std::string dbName = "db_test";

struct AltChainParamsTest : public AltChainParams {
  AltBlock getGenesisBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }
};

struct RewardsTestFixture : ::testing::Test {
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;

  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<VbkChainParams> vbk_params;
  std::shared_ptr<AltChainParams> altChainParams;
  PopRewardsParams rewardParams{};

  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> btcTree;
  std::shared_ptr<VbkBlockTree> vbkTree;
  std::shared_ptr<BtcTree> altTree;

  std::shared_ptr<EndorsementRepository<BtcEndorsement>> erepo;
  std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbk_erepo;

  std::shared_ptr<MockMiner> apm;
  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> alt_miner;

  std::shared_ptr<PopManager> altpop;

  ValidationState state;

  std::shared_ptr<PopRewardsCalculator> rewardsCalculator;
  std::shared_ptr<PopRewards> rewards;

  StateManager<RepositoryRocksManager> stateManager;

  void setUpChains() {
    ASSERT_TRUE(apm->btc().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(apm->vbk().bootstrapWithGenesis(state));
    ASSERT_TRUE(state.IsValid());    

    EXPECT_TRUE(altpop->btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(altpop->vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(altTree->bootstrapWithGenesis(state));
    EXPECT_TRUE(state.IsValid());
  }

  RewardsTestFixture() : stateManager(dbName) {
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    vbk_params = std::make_shared<VbkChainParamsRegTest>();
    altChainParams = std::make_shared<AltChainParamsTest>();

    btcTree = std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btc_params);
    vbkTree = std::make_shared<VbkBlockTree>(*btcTree, erepo, vbk_params);

    erepo = std::make_shared<EndorsementRepositoryInmem<BtcEndorsement>>();
    vbk_erepo = std::make_shared<EndorsementRepositoryInmem<VbkEndorsement>>();

    apm = std::make_shared<MockMiner>();

    rewardsCalculator =
        std::make_shared<PopRewardsCalculator>(*altChainParams, rewardParams);
    rewards = std::make_shared<PopRewards>(
        *vbk_erepo, *vbkTree, rewardParams, *rewardsCalculator);

    altTree = std::make_shared<BtcTree>(btc_params);

    altpop = std::make_shared<PopManager>(
        btc_params, vbk_params, erepo, vbk_erepo, altChainParams);

    setUpChains();
  }

  template <typename Tree>
  void mineChain(Tree& tree,
                 std::vector<typename Tree::block_t>& blocks,
                 int size) {
    std::generate_n(
        std::back_inserter(blocks), size, [&]() -> typename Tree::block_t {
          auto* tip = tree.getBestChain().tip();
          EXPECT_NE(tip, nullptr);
          auto block = alt_miner->createNextBlock(*tip);
          EXPECT_TRUE(tree.acceptBlock(block, state));
          return block;
        });
  }

  const std::vector<uint8_t> payoutInfo{1, 2, 3, 4, 5};

  PublicationData endorseBlock(const std::vector<uint8_t>& block) {
    PublicationData pub;
    pub.header = block;
    pub.identifier = 1;
    pub.contextInfo = {};
    pub.payoutInfo = payoutInfo;
    return pub;
  }

  AltBlock makeAltBlock(const BlockIndex<BtcBlock>& block) {
    AltBlock ret;
    ret.timestamp = block.getBlockTime();
    ret.height = block.height;
    ret.hash = block.getHash().asVector();
    return ret;
  }
};

TEST_F(RewardsTestFixture, basicReward_test) {
  PopRewardsBigDecimal popDifficulty = 1.0;
  AltBlock endorsedBlock = {};

  EXPECT_TRUE(apm->mineBtcBlocks(5, state));
  EXPECT_EQ(apm->btc().getBestChain().tip()->height, 5);

  EXPECT_TRUE(apm->mineVbkBlocks(5, state));
  EXPECT_EQ(apm->vbk().getBestChain().tip()->height, 5);

  // ALT has genesis + 10 blocks
  std::vector<BtcBlock> altfork1{btc_params->getGenesisBlock()};
  mineChain(*altTree, altfork1, 10);
  ASSERT_EQ(altTree->getBestChain().chainHeight(), 10);

  // get last known BTC and VBK hashes (current tips)
  auto last_vbk = apm->vbk().getBestChain().tip()->getHash();
  auto last_btc = apm->btc().getBestChain().tip()->getHash();

  // endorse ALT tip, at height 10
  auto* endorsedAltBlockIndex = altTree->getBestChain().tip();
  PublicationData pub = endorseBlock(endorsedAltBlockIndex->header.toRaw());

  // mine publications
  auto [atv, vtbs] = apm->mine(pub, last_vbk, last_btc, 1, state);
  ASSERT_TRUE(state.IsValid()) << state.GetRejectReason();

  // mine alt block, which CONTAINS this alt pop tx with endorsement
  mineChain(*altTree, altfork1, 1);

  AltProof altProof;
  altProof.atv = atv;
  // endorsed ALT block
  altProof.endorsed = makeAltBlock(*endorsedAltBlockIndex);
  // block that contains current "payloads"
  altProof.containing = makeAltBlock(*altTree->getBestChain().tip());

  // apply payloads to our current view
  std::unique_ptr<StateChange> change = stateManager.newChange();
  ASSERT_TRUE(altpop->addPayloads({altProof, vtbs, {}, {}}, *change, state))
      << state.GetRejectReason();

  auto payouts = rewards->calculatePayouts(altProof.endorsed, popDifficulty);
  ASSERT_TRUE(payouts.size() > 0);
}
