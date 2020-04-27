// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

struct PopPayoutsE2Etest : public ::testing::Test, public PopTestFixture {};

TEST_F(PopPayoutsE2Etest, AnyBlockCanBeAccepted_NoEndorsements) {
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};

  for (size_t i = 0; i < 10000; i++) {
    std::map<std::vector<uint8_t>, int64_t> payout;
    ASSERT_NO_FATAL_FAILURE(payout = alttree.getPopPayout(chain[i].getHash()));
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
  auto vbktx = popminer.endorseAltBlock(data);
  popminer.mineVbkBlocks(1);
  auto containing = generateNextBlock(chain.back());
  chain.push_back(containing);
  auto payloads =
      generateAltPayloads(vbktx, containing, endorsed, getLastKnownVbkBlock());

  auto payout = alttree.getPopPayout(chain[0].getHash());
  ASSERT_TRUE(payout.empty());
  ASSERT_TRUE(alttree.acceptBlock(containing, state));
  ASSERT_TRUE(alttree.addPayloads(containing, {payloads}, state));

  // endorsed.height=0, containing.height=1
  payout = alttree.getPopPayout(chain[0].getHash());
  ASSERT_FALSE(payout.empty());
  ASSERT_TRUE(payout.count(getPayoutInfo()));
  ASSERT_GT(payout[getPayoutInfo()], 0);
}