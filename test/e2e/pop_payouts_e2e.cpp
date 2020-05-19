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

  AltPayloads generateAltPayloads2(MockMiner& miner,
                                   const VbkTx& transaction,
                                   const AltBlock& containing,
                                   const AltBlock& endorsed,
                                   const VbkBlock::hash_t& lastVbk,
                                   int VTBs = 0) {
    PopData popData;

    for (auto i = 0; i < VTBs; i++) {
      auto vbkpoptx = generatePopTx(getLastKnownVbkBlock());
      auto vbkcontaining = miner.applyVTB(miner.vbk(), vbkpoptx, state);
      auto newvtb = miner.vbkPayloads.at(vbkcontaining.getHash()).back();
      popData.vtbs.push_back(newvtb);
    }

    popData.hasAtv = true;
    popData.atv = miner.generateATV(transaction, lastVbk, state);

    fillVbkContext(popData.vbk_context,
                   lastVbk,
                   popData.atv.containingBlock.getHash(),
                   miner.vbk());

    AltPayloads alt;
    alt.popData = popData;
    alt.containingBlock = containing;
    alt.endorsed = endorsed;

    return alt;
  }

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
  auto data = generatePublicationData(endorsed);
  auto vbktx = popminer.createVbkTxEndorsingAltBlock(data);
  popminer.mineVbkBlocks(1);
  auto containing = generateNextBlock(chain.back());
  chain.push_back(containing);
  auto payloads =
      generateAltPayloads(vbktx, containing, endorsed, getLastKnownVbkBlock());

  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_TRUE(payout.empty());
  ASSERT_TRUE(state.IsError());

  state = ValidationState();
  ASSERT_TRUE(alttree.acceptBlock(containing, state));
  ASSERT_TRUE(alttree.addPayloads(containing, {payloads}, state));
  ASSERT_TRUE(alttree.setState(containing.hash, state));

  mineAltBlocksWithTree(
      alttree,
      altparam.getRewardParams().rewardSettlementInterval() - 2,
      chain);

  payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_FALSE(payout.empty());
  ASSERT_TRUE(payout.count(getPayoutInfo()));
  ASSERT_GT(payout[getPayoutInfo()], 0);
}

/*
 * Scenario 1. Two endorsements in blocks N and N + 1 endorsing
 * the same ALT block share the same reward.
 */
TEST_F(PopPayoutsE2Etest, ManyEndorsementsSameReward) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  auto endorsed = chain.back();

  for (int i = 0; i < 2; i++) {
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx = popminer.createVbkTxEndorsingAltBlock(data);
    popminer.mineVbkBlocks(1);
    auto containing = generateNextBlock(chain.back());
    chain.push_back(containing);
    auto payloads = generateAltPayloads(
        vbktx, containing, endorsed, getLastKnownVbkBlock());

    state = ValidationState();
    ASSERT_TRUE(alttree.acceptBlock(containing, state));
    ASSERT_TRUE(alttree.addPayloads(containing, {payloads}, state));
    ASSERT_TRUE(alttree.setState(containing.hash, state));
  }

  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_TRUE(payout.empty());
  ASSERT_TRUE(state.IsError());

  state = ValidationState();
  mineAltBlocksWithTree(
      alttree,
      altparam.getRewardParams().rewardSettlementInterval() - 3,
      chain);

  payout = alttree.getPopPayout(chain.back().getHash(), state);
  ASSERT_EQ(payout.size(), 2);
  auto miner1 = getPayoutInfo();
  miner1.push_back(0);
  auto miner2 = getPayoutInfo();
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
  for (size_t i = 0;
       i < (altparam.getRewardParams().rewardSettlementInterval() + 102);
       i++) {
    auto endorsed = chain.back();
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(i >> 8));
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx = popminer.createVbkTxEndorsingAltBlock(data);
    popminer.mineVbkBlocks(1);
    auto containing = generateNextBlock(chain.back());
    chain.push_back(containing);
    auto payloads = generateAltPayloads(
        vbktx, containing, endorsed, getLastKnownVbkBlock());
    ASSERT_TRUE(alttree.acceptBlock(containing, state));
    ASSERT_TRUE(alttree.addPayloads(containing, {payloads}, state));
    ASSERT_TRUE(alttree.setState(containing.hash, state));
  }

  // wait for the reward
  mineAltBlocksWithTree(
      alttree, altparam.getRewardParams().rewardSettlementInterval() - 2, chain);

  // this is a regular payout - each block is endorsed by the next one
  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  auto firstBlock =
      alttree.getBlockIndex(chain.back().getHash())
          ->getAncestorBlocksBehind(
              altparam.getRewardParams().rewardSettlementInterval());

  state = ValidationState();
  MockMiner popminer2{};
  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam);
  EXPECT_TRUE(alttree2.bootstrap(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.vbk().btc().bootstrapWithGenesis(state));

  std::vector<AltBlock> chain2{altparam.getBootstrapBlock()};
  for (size_t i = 0;
       i < altparam.getRewardParams().rewardSettlementInterval();
       i++) {
    auto endorsed = chain2.back();
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(i >> 8));
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx = popminer2.createVbkTxEndorsingAltBlock(data);
    popminer2.mineVbkBlocks(1);
    auto containing = generateNextBlock(chain2.back());
    chain2.push_back(containing);
    auto payloads = generateAltPayloads2(popminer2,
                             vbktx,
                             containing,
                             endorsed,
                             alttree2.vbk().getBestChain().tip()->getHash());
    ASSERT_TRUE(alttree2.acceptBlock(containing, state));
    ASSERT_TRUE(alttree2.addPayloads(containing, {payloads}, state));
    ASSERT_TRUE(alttree2.setState(containing.hash, state));
  }

  mineAltBlocksWithTree(alttree2, 101, chain2);
  auto endorsedBlock = chain2.back();

  auto data = generatePublicationData(endorsedBlock);
  data.payoutInfo.push_back(10);
  data.payoutInfo.push_back(100);
  auto vbktx = popminer2.createVbkTxEndorsingAltBlock(data);
  popminer2.mineVbkBlocks(1);
  auto containing = generateNextBlock(chain2.back());
  chain2.push_back(containing);
  auto payloads = generateAltPayloads2(popminer2,
                           vbktx,
                           containing,
                           endorsedBlock,
                           alttree2.vbk().getBestChain().tip()->getHash());

  ASSERT_TRUE(alttree2.acceptBlock(containing, state));
  ASSERT_TRUE(alttree2.addPayloads(containing, {payloads}, state));
  ASSERT_TRUE(alttree2.setState(containing.hash, state));

  mineAltBlocksWithTree(
      alttree2,
      altparam.getRewardParams().rewardSettlementInterval() - 2,
      chain2);
  auto payout2 = alttree2.getPopPayout(chain2.back().getHash(), state);
  auto secondBlock =
      alttree2.getBlockIndex(chain2.back().getHash())
          ->getAncestorBlocksBehind(
              altparam.getRewardParams().rewardSettlementInterval());

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
  for (size_t i = 0;
       i < altparam.getRewardParams().rewardSettlementInterval() + 1;
       i++) {
    auto endorsed = chain.back();
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(i >> 8));
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx1 = popminer.createVbkTxEndorsingAltBlock(data);
    popminer.mineVbkBlocks(1);
    data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(i >> 8) + 10);
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx2 = popminer.createVbkTxEndorsingAltBlock(data);
    popminer.mineVbkBlocks(1);
    auto containing = generateNextBlock(chain.back());
    chain.push_back(containing);
    auto payloads1 = generateAltPayloads(
        vbktx1, containing, endorsed, getLastKnownVbkBlock());
    auto payloads2 = generateAltPayloads(
        vbktx2, containing, endorsed, getLastKnownVbkBlock());
    ASSERT_TRUE(alttree.acceptBlock(containing, state));
    ASSERT_TRUE(alttree.addPayloads(containing, {payloads1, payloads2}, state));
    ASSERT_TRUE(alttree.setState(containing.hash, state));
  }

  auto endorsed1 = chain.back();
  auto data1 = generatePublicationData(endorsed1);
  // change reward recipient so we can get distinct rewards
  data1.payoutInfo.push_back(10);
  data1.payoutInfo.push_back(1);
  auto vbktx1 = popminer.createVbkTxEndorsingAltBlock(data1);
  popminer.mineVbkBlocks(1);
  auto containing1 = generateNextBlock(chain.back());
  chain.push_back(containing1);
  auto payloads1 =
      generateAltPayloads(vbktx1, containing1, endorsed1, getLastKnownVbkBlock());
  ASSERT_TRUE(alttree.acceptBlock(containing1, state));
  ASSERT_TRUE(alttree.addPayloads(containing1, {payloads1}, state));
  ASSERT_TRUE(alttree.setState(containing1.hash, state));

  // wait for the reward
  mineAltBlocksWithTree(
      alttree, altparam.getRewardParams().rewardSettlementInterval() - 2, chain);

  // each block is endorsed by the next one but we have higher difficulty
  // since before each block was endorsed by two miners
  auto payout = alttree.getPopPayout(chain.back().getHash(), state);
  auto firstBlock =
      alttree.getBlockIndex(chain.back().getHash())
          ->getAncestorBlocksBehind(
              altparam.getRewardParams().rewardSettlementInterval());

  state = ValidationState();
  MockMiner popminer2{};
  AltTree alttree2 = AltTree(altparam, vbkparam, btcparam);
  EXPECT_TRUE(alttree2.bootstrap(state));
  EXPECT_TRUE(alttree2.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree2.vbk().btc().bootstrapWithGenesis(state));

  std::vector<AltBlock> chain2{altparam.getBootstrapBlock()};
  for (size_t i = 0; i < altparam.getRewardParams().rewardSettlementInterval() + 2;
       i++) {
    auto endorsed = chain2.back();
    auto data = generatePublicationData(endorsed);
    // change reward recipient so we can get distinct rewards
    data.payoutInfo.push_back((uint8_t)(i >> 8));
    data.payoutInfo.push_back((uint8_t)i);
    auto vbktx = popminer2.createVbkTxEndorsingAltBlock(data);
    popminer2.mineVbkBlocks(1);
    auto containing = generateNextBlock(chain2.back());
    chain2.push_back(containing);
    auto payloads =
        generateAltPayloads2(popminer2,
                             vbktx,
                             containing,
                             endorsed,
                             alttree2.vbk().getBestChain().tip()->getHash());
    ASSERT_TRUE(alttree2.acceptBlock(containing, state));
    ASSERT_TRUE(alttree2.addPayloads(containing, {payloads}, state));
    ASSERT_TRUE(alttree2.setState(containing.hash, state));
  }

  // wait for the reward
  mineAltBlocksWithTree(
      alttree2,
      altparam.getRewardParams().rewardSettlementInterval() - 2,
      chain2);

  auto payout2 = alttree2.getPopPayout(chain2.back().getHash(), state);
  auto secondBlock =
      alttree2.getBlockIndex(chain2.back().getHash())
          ->getAncestorBlocksBehind(
              altparam.getRewardParams().rewardSettlementInterval());

  // make sure this endorsed block is at the same height as previous
  // endorsed block
  ASSERT_EQ(firstBlock->height, secondBlock->height);
  ASSERT_EQ(payout2.size(), payout.size());
  ASSERT_GT(payout2.begin()->second, payout.begin()->second);
}
