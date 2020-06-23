// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>

using namespace altintegration;

std::basic_stringstream<char>& operator<<(
    std::basic_stringstream<char>& os,
    const std::map<std::vector<uint8_t>, int64_t>& v) {
  os << "size: " << v.size() << "\n";
  for (const auto& o : v) {
    os << HexStr(o.first) << ": " << o.second << "\n";
  }
  return os;
}

struct PopPayoutsE2Etest : public ::testing::Test, public PopTestFixture {
  PopPayoutsE2Etest() : PopTestFixture(), rewards_(altparam) {}

  void mineAltBlocksWithTree(AltTree& tree,
                             uint32_t num,
                             std::vector<AltBlock>& chain) {
    ASSERT_NE(chain.size(), 0);

    for (uint32_t i = 0; i < num; ++i) {
      chain.push_back(generateNextBlock(*chain.rbegin()));

      ASSERT_TRUE(tree.acceptBlock(*chain.rbegin(), state));
      ASSERT_TRUE(tree.setState(chain.rbegin()->getHash(), state));
      ASSERT_TRUE(state.IsValid());
    }
  }

  void mineSingleEndorsement(AltTree& tree,
                             AltBlock& endorsed,
                             size_t num,
                             std::vector<AltBlock>& chain) {
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(num >> 8));
    data.payoutInfo.push_back((uint8_t)num);
    auto vbktx = popminer->createVbkTxEndorsingAltBlock(data);
    popminer->mineVbkBlocks(1);
    auto containing = generateNextBlock(chain.back());
    chain.push_back(containing);
    auto payloads = generateAltPayloads(
        {vbktx}, tree.vbk().getBestChain().tip()->getHash());
    ASSERT_TRUE(tree.acceptBlock(containing, state));
    ASSERT_TRUE(tree.addPayloads(containing, payloads, state));
    ASSERT_TRUE(tree.setState(containing.hash, state));
  }

  void mineEndorsements(AltTree& tree,
                        size_t num,
                        std::vector<AltBlock>& chain) {
    for (size_t i = 0; i < num; i++) {
      auto endorsed = chain.back();
      mineSingleEndorsement(tree, endorsed, i, chain);
    }
  }

  void mineTwoEndorsements(AltTree& tree,
                           size_t num,
                           std::vector<AltBlock>& chain) {
    for (size_t i = 0; i < num; i++) {
      auto endorsed = chain.back();
      auto data = generatePublicationData(endorsed);
      // change reward recipient so we can get distinct rewards
      data.payoutInfo.push_back((uint8_t)(i >> 8));
      data.payoutInfo.push_back((uint8_t)i);
      auto vbktx1 = popminer->createVbkTxEndorsingAltBlock(data);

      data = generatePublicationData(endorsed);
      // change reward recipient so we can get distinct rewards
      data.payoutInfo.push_back((uint8_t)(i >> 8) + 10);
      data.payoutInfo.push_back((uint8_t)i);
      auto vbktx2 = popminer->createVbkTxEndorsingAltBlock(data);

      popminer->mineVbkBlocks(1);
      auto containing = generateNextBlock(chain.back());
      chain.push_back(containing);
      auto payloads1 = generateAltPayloads(
          {vbktx1, vbktx2}, tree.vbk().getBestChain().tip()->getHash());
      ASSERT_TRUE(tree.acceptBlock(containing, state));
      ASSERT_TRUE(tree.addPayloads(containing, payloads1, state));
      ASSERT_TRUE(tree.setState(containing.hash, state));
    }
  }

  PopRewardsCalculator rewards_;
};

TEST_F(PopPayoutsE2Etest, AnyBlockCanBeAccepted_NoEndorsements) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};

  for (size_t i = 0; i < 10000; i++) {
    std::map<std::vector<uint8_t>, int64_t> payout;
    ASSERT_NO_FATAL_FAILURE(
        payout = alttree.getPopPayout(chain[i].getHash(), state));
    // no endorsements = no payouts
    ASSERT_TRUE(payout.empty());

    auto block = generateNextBlock(chain.back());
    chain.push_back(block);
    ASSERT_TRUE(alttree.acceptBlock(block, state));
  }
}

TEST_F(PopPayoutsE2Etest, OnePayout) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  auto endorsed = chain.back();
  mineSingleEndorsement(alttree, endorsed, 1, chain);

  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_TRUE(payout.empty());
  ASSERT_TRUE(state.IsError());

  state = ValidationState();
  mineAltBlocksWithTree(
      alttree, altparam.getEndorsementSettlementInterval() - 2, chain);

  payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_FALSE(payout.empty());

  auto miner1 = getPayoutInfo();
  miner1.push_back(0);
  miner1.push_back(1);
  ASSERT_TRUE(payout.count(miner1));
  ASSERT_GT(payout[miner1], 0);
}

/*
 * Scenario 1. Two endorsements in blocks N and N + 1 endorsing
 * the same ALT block share the same reward.
 */
TEST_F(PopPayoutsE2Etest, ManyEndorsementsSameReward) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  auto endorsed = chain.back();
  for (int i = 0; i < 2; i++) {
    mineSingleEndorsement(alttree, endorsed, i, chain);
  }

  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_TRUE(payout.empty());
  ASSERT_TRUE(state.IsError());

  state = ValidationState();
  mineAltBlocksWithTree(
      alttree, altparam.getEndorsementSettlementInterval() - 3, chain);

  payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_EQ(payout.size(), 2);
  auto miner1 = getPayoutInfo();
  miner1.push_back(0);
  miner1.push_back(0);
  auto miner2 = getPayoutInfo();
  miner2.push_back(0);
  miner2.push_back(1);
  ASSERT_NE(miner1, miner2);
  ASSERT_EQ(payout[miner1], payout[miner2]);
  ASSERT_GT(payout[miner1], 0);
}

/*
 * Scenario 2. Have 101 block without endorsements and make sure the reward
 * for block at round #1 is the same as during normal endorsements mining.
 */
TEST_F(PopPayoutsE2Etest, SameRewardWhenNoEndorsements) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  mineEndorsements(
      alttree, altparam.getEndorsementSettlementInterval() + 102, chain);

  // wait for the reward
  mineAltBlocksWithTree(
      alttree, altparam.getEndorsementSettlementInterval() - 2, chain);

  // this is a regular payout - each block is endorsed by the next one
  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  auto firstBlock = alttree.getBlockIndex(chain.back().getHash())
                        ->getAncestorBlocksBehind(
                            altparam.getEndorsementSettlementInterval());

  state = ValidationState();
  popminer = std::make_shared<MockMiner>();
  std::vector<AltBlock> chain2{altparam.getBootstrapBlock()};
  PayloadsStorage storage2{};
  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam, storage2);
  EXPECT_TRUE(alttree2.bootstrap(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.vbk().btc().bootstrapWithGenesis(state));

  mineEndorsements(
      alttree2, altparam.getEndorsementSettlementInterval(), chain2);
  mineAltBlocksWithTree(alttree2, 101, chain2);

  auto endorsedBlock = chain2.back();
  mineSingleEndorsement(alttree2, endorsedBlock, 10000, chain2);
  mineAltBlocksWithTree(
      alttree2, altparam.getEndorsementSettlementInterval() - 2, chain2);

  auto payout2 = alttree2.getPopPayout(chain2.back().getHash(), state);
  auto secondBlock = alttree2.getBlockIndex(chain2.back().getHash())
                         ->getAncestorBlocksBehind(
                             altparam.getEndorsementSettlementInterval());

  // make sure this endorsed block is at the same height as previous
  // endorsed block
  ASSERT_EQ(firstBlock->height, secondBlock->height);

  // we can see that despite we had 101 block without endorsements,
  // rewards stays the same
  ASSERT_EQ(payout2.size(), payout.size());
  ASSERT_EQ(payout2.begin()->second, payout.begin()->second);
}

/*
 * Scenario 3. Prepare a chain A with two endorsements for each block and
 * a chain B with one endorsement for each block. Wait for the reward
 * of the last block on each chain and make sure that chain B reward
 * is higher than chain A reward.
 */
TEST_F(PopPayoutsE2Etest, GrowingRewardWhenLessMiners) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};

  // prepare chain where each block is endorsed by two miners
  mineTwoEndorsements(
      alttree, altparam.getEndorsementSettlementInterval() + 1, chain);
  auto endorsed1 = chain.back();
  mineSingleEndorsement(alttree, endorsed1, 10000, chain);

  // wait for the reward
  mineAltBlocksWithTree(
      alttree, altparam.getEndorsementSettlementInterval() - 2, chain);

  // each block is endorsed by the next one but we have higher difficulty
  // since before each block was endorsed by two miners
  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  auto firstBlock = alttree.getBlockIndex(chain.back().getHash())
                        ->getAncestorBlocksBehind(
                            altparam.getEndorsementSettlementInterval());

  state = ValidationState();
  popminer = std::make_shared<MockMiner>();
  std::vector<AltBlock> chain2{altparam.getBootstrapBlock()};
  PayloadsStorage storage2{};
  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam, storage2);
  EXPECT_TRUE(alttree2.vbk().btc().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.bootstrap(state));

  mineEndorsements(
      alttree2, altparam.getEndorsementSettlementInterval() + 2, chain2);
  // wait for the reward
  mineAltBlocksWithTree(
      alttree2, altparam.getEndorsementSettlementInterval() - 2, chain2);

  auto payout2 = alttree2.getPopPayout(chain2.back().getHash(), state);
  auto secondBlock = alttree2.getBlockIndex(chain2.back().getHash())
                         ->getAncestorBlocksBehind(
                             altparam.getEndorsementSettlementInterval());

  // make sure this endorsed block is at the same height as previous
  // endorsed block
  ASSERT_EQ(firstBlock->height, secondBlock->height);
  ASSERT_EQ(payout2.size(), payout.size());
  ASSERT_GT(payout2.begin()->second, payout.begin()->second);
}

/*
 * Scenario 4. Wait for payout blocks and make sure keystone block has
 * the highest reward.
 */
TEST_F(PopPayoutsE2Etest, HigherRewardForKeystone) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  mineEndorsements(alttree, altparam.getKeystoneInterval() * 2, chain);

  // wait for the reward
  mineAltBlocksWithTree(
      alttree, altparam.getEndorsementSettlementInterval() - 2, chain);

  int64_t highestReward = 0;
  int blockNumber = 0;
  auto initialBlock = alttree.getBlockIndex(chain.back().getHash());

  // find maximum reward and store it together with endorsed block height
  for (size_t i = 0; i < altparam.getKeystoneInterval(); i++) {
    auto payout = alttree.getPopPayout(initialBlock->getHash(), state);
    if (payout.begin()->second > highestReward) {
      highestReward = payout.begin()->second;
      auto endorsedBlock = initialBlock->getAncestorBlocksBehind(
          altparam.getEndorsementSettlementInterval());
      blockNumber = endorsedBlock->height;
    }
    initialBlock = initialBlock->pprev;
  }

  auto roundNumber = rewards_.getRoundForBlockNumber(blockNumber);
  ASSERT_EQ(roundNumber,
            rewards_.getAltParams().getRewardParams().keystoneRound());
  ASSERT_GT(highestReward, 0);
}
