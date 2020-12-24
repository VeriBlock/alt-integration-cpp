// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/alt-util.hpp>
#include <veriblock/mock_miner.hpp>
#include <util/alt_chain_params_regtest.hpp>

using namespace altintegration;

struct AltUtilTest : public ::testing::Test {
  MockMiner popminer;
};

TEST_F(AltUtilTest, GetLastKnownBlocks) {
  auto genesis = getLastKnownBlocks(popminer.btc(), 1ull);
  ASSERT_EQ(genesis.size(), 1);
  ASSERT_EQ(genesis[0], GetRegTestBtcBlock().getHash().asVector());

  auto a = popminer.mineBtcBlocks(10);
  auto one = getLastKnownBlocks(popminer.btc(), 1);
  ASSERT_EQ(one.size(), 1);
  ASSERT_EQ(one[0], a->getHash().asVector());

  auto ten = getLastKnownBlocks(popminer.btc(), 10ull);
  ASSERT_EQ(ten.size(), 10);
  for (size_t i = 0; i < 10; i++) {
    ASSERT_EQ(ten[i], a->getAncestor((int)(1 + i))->getHash().asVector());
  }

  auto many = getLastKnownBlocks(popminer.btc(), 1000ull);
  ASSERT_EQ(many.size(), 11ull);  // we know only 11 bocks, 10+genesis
}

TEST(TopLevelMerkleRoot, Algorithm) {
  AltChainParamsRegTest p;
  uint256 txRoot = uint256::fromHex("01");
  uint256 popDataRoot = uint256::fromHex("02");

  ContextInfoContainer container;
  container.height = 10;
  container.keystones.firstPreviousKeystone = ParseHex("03");
  container.keystones.secondPreviousKeystone = ParseHex("04");

  auto left = sha256twice(txRoot, popDataRoot);
  auto right = container.getHash();
  auto topLevel = sha256twice(left, right);

  AuthenticatedContextInfoContainer c;
  c.stateRoot = left;
  c.ctx = container;

  ASSERT_EQ(topLevel, c.getTopLevelMerkleRoot());
  ASSERT_EQ(topLevel, CalculateTopLevelMerkleRoot(txRoot, popDataRoot, container));
  ASSERT_EQ(topLevel, CalculateTopLevelMerkleRoot(c));
}