#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "veriblock/rewards/poprewards_calculator.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "util/pop_test_fixture.hpp"

using namespace altintegration;

///TODO: uncomment when we are ready to test rewards
#if 0

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

struct RewardsTestFixture : ::testing::Test {
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;

  ///HACK: we use BTC tree to emulate AltChain tree
  std::shared_ptr<BtcTree> altTree;

  PopRewardsParams reward_params{};
  std::shared_ptr<AltChainParams> alt_params;

  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> alt_miner;

  MockMiner popminer;

  std::shared_ptr<PopRewardsCalculator> rewardsCalculator;
  std::shared_ptr<PopRewards> rewards;

  ValidationState state;

  RewardsTestFixture() {
    alt_params = std::make_shared<AltChainParamsTest>();
    alt_miner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(popminer.btc().getParams());

    rewardsCalculator =
        std::make_shared<PopRewardsCalculator>(*alt_params, reward_params);
    rewards = std::make_shared<PopRewards>(popminer.vbk(), reward_params, *rewardsCalculator);

    altTree = std::make_shared<BtcTree>(popminer.btc().getParams());
    EXPECT_TRUE(altTree->bootstrapWithGenesis(state));
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
  std::vector<BtcBlock> altfork1{popminer.btc().getParams().getGenesisBlock()};
  mineChain(*altTree, altfork1, 10);
  // ALT has genesis + 10 blocks
  ASSERT_EQ(altTree->getBestChain().chainHeight(), 10);

  // endorse ALT tip, at height 10
  auto* endorsedAltBlockIndex = altTree->getBestChain().tip();
  PublicationData pub = endorseBlock(endorsedAltBlockIndex->header.toRaw());

  // mine publications
  auto [atv, vtbs] = apm->mine(pub, last_vbk, last_btc, 1, state);
  ASSERT_TRUE(state.IsValid()) << state.GetPath();

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
      << state.GetPath();

  PopRewardsBigDecimal popDifficulty = 1.0;
  auto payouts = rewards->calculatePayouts(altProof.endorsed, popDifficulty);
  ASSERT_TRUE(payouts.size() == 1);
}

#endif //0
