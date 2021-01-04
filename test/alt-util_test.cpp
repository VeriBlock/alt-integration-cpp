// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>
#include <veriblock/mock_miner_2.hpp>

using namespace altintegration;

struct AltUtilTest : public ::testing::Test {
  MockMiner2 popminer;
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
  ASSERT_EQ(
      topLevel,
      CalculateTopLevelMerkleRoot(txRoot.asVector(), popDataRoot, container));
  ASSERT_EQ(topLevel, CalculateTopLevelMerkleRoot(c));
}

TEST(EmptyPopData, MerkleRoot) {
  PopData p;
  p.version = 1;
  auto root = p.getMerkleRoot();
  ASSERT_EQ(HexStr(root),
            "eaac496a5eab315c9255fb85c871cef7fd87047adcd2e81ba7d55d6bdeb1737f");
}

TEST(TopLevelMerkleRoot, Sanity) {
  ContextInfoContainer ctx;
  ctx.height = 1337;
  ctx.keystones.firstPreviousKeystone = std::vector<uint8_t>{1, 2, 3};
  ctx.keystones.secondPreviousKeystone = std::vector<uint8_t>{4, 5, 6};

  WriteStream w;
  ctx.toVbkEncoding(w);
  ASSERT_EQ(HexStr(w.data()), "000005390301020303040506");
  ASSERT_EQ(HexStr(ctx.getHash()),
            "db35aad09a65b667a6c9e09cbd47b8d6b378b9ec705db604a4d5cd489afd2bc6");

  std::vector<uint8_t> txRoot = ParseHex(
      "bf9fb4901a0d8fc9b0d3bf38546191f77a3f2ea5d543546aac0574290c0a9e83");
  PopData p;
  p.version = 1;
  auto popDataRoot = p.getMerkleRoot();

  auto stateRoot = sha256twice(txRoot, popDataRoot);
  ASSERT_EQ(HexStr(stateRoot),
            "e9e547df56daa0ea36de14b923aa2efd9ad09c6207e6304584d900904a03596a");

  auto tlmr = CalculateTopLevelMerkleRoot(txRoot, popDataRoot, ctx);
  ASSERT_EQ(HexStr(tlmr),
            "700c1abb69dd1899796b4cafa81c0eefa7b7d0c5aaa4b2bcb67713b2918edb52");
}