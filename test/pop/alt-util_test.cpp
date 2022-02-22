// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>
#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/mock_miner.hpp>

using namespace altintegration;

struct AltUtilTest : public ::testing::Test {
  BtcChainParamsRegTest btc_params;
  VbkChainParamsRegTest vbk_params;
  AltChainParamsRegTest alt_params;

  MockMiner popminer{alt_params, vbk_params, btc_params};
};

TEST(AltUtil, MaxAtvsInVbk) {
  EXPECT_EQ(getMaxAtvsInVbkBlock(0), 2147483647);
  EXPECT_EQ(getMaxAtvsInVbkBlock(0xFF), 1);
  EXPECT_EQ(getMaxAtvsInVbkBlock(0x06FF), 4);
  EXPECT_EQ(getMaxAtvsInVbkBlock(0x05FF), 9);
  EXPECT_EQ(getMaxAtvsInVbkBlock((0b10011 << 8) + 0xff), 100);  // (9+1)*(9+1)
  EXPECT_EQ(getMaxAtvsInVbkBlock((0b101000 << 8) + 0xff), 21);  // 20+1
  EXPECT_EQ(getMaxAtvsInVbkBlock((0b100110 << 8) + 0xff), 20);  // 19+1
  EXPECT_EQ(getMaxAtvsInVbkBlock(0x26ff), 20);
}

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

// ~ » vbitcoin-cli getblocktemplate "{\"rules\": [\"segwit\"]}"
//{
//  "capabilities": [
//    "proposal"
//  ],
//  "version": 536870912,
//  "rules": [
//  ],
//  "vbavailable": {
//  },
//  "vbrequired": 0,
//  "previousblockhash":
//  00000000115f2f3286a442fe32aaf39abefa6fa1e5a4d7e5661cf2c6c664e0eb",
//  "transactions": [
//  ],
//  "coinbaseaux": {
//  },
//  "coinbasevalue": 3000000000,
//  "longpollid":
//  00000000115f2f3286a442fe32aaf39abefa6fa1e5a4d7e5661cf2c6c664e0eb3754",
//  "target":
//  0000001217020000000000000000000000000000000000000000000000000000",
//  "mintime": 1611315289,
//  "mutable": [
//    "time",
//    "transactions",
//    "prevblock"
//  ],
//  "noncerange": "00000000ffffffff",
//  "sigoplimit": 80000,
//  "sizelimit": 4000000,
//  "weightlimit": 4000000,
//  "curtime": 1611315938,
//  "bits": "1d121702",
//  "height": 4207,
//  "default_witness_commitment":
//  6a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf9",
//  "pop_data_root":
//  eaac496a5eab315c9255fb85c871cef7fd87047adcd2e81ba7d55d6bdeb1737f",
//  "tx_root":
//  31deca6d182a7be32cc5edf0ac6724be82ae41afc363abf35d715755ff4a0ba2",
//  "pop_data": {
//    "version": 1,
//    "vbkblocks": [
//    ],
//    "vtbs": [
//    ],
//    "atvs": [
//    ]
//  },
//  "pop_context": {
//    "serialized":
//    0000106f20e665f741498cf23a4abfc5e52c675307ae7b28ea75cda72a48af89de0200000020714233a5a9c3a984c621ebdfd188dba90fccc47b162a648c1f2929d90c000000e68be583d1048006108729c02781c228afe9a1b785d067692054bb39d1658ddc",
//    "stateRoot":
//    e68be583d1048006108729c02781c228afe9a1b785d067692054bb39d1658ddc",
//    "context": {
//      "height": 4207,
//      "firstPreviousKeystone":
//      e665f741498cf23a4abfc5e52c675307ae7b28ea75cda72a48af89de02000000",
//      "secondPreviousKeystone":
//      714233a5a9c3a984c621ebdfd188dba90fccc47b162a648c1f2929d90c000000"
//    }
//  },
//  "pop_rewards": [
//  ]
//}
TEST(TopLevelMerkleRoot, Case1) {
  // data that we got from getblocktemplate
  // clang-format off
  const int height = 4207;
  const std::vector<uint8_t> firstPreviousKeystone = ParseHex("e665f741498cf23a4abfc5e52c675307ae7b28ea75cda72a48af89de02000000");
  const std::vector<uint8_t> secondPreviousKeystone = ParseHex("714233a5a9c3a984c621ebdfd188dba90fccc47b162a648c1f2929d90c000000");
  const std::vector<uint8_t> tx_root = ParseHex("31deca6d182a7be32cc5edf0ac6724be82ae41afc363abf35d715755ff4a0ba2");
  const std::vector<uint8_t> pop_data_root = ParseHex("eaac496a5eab315c9255fb85c871cef7fd87047adcd2e81ba7d55d6bdeb1737f");
  const std::vector<uint8_t> pop_context_serialized = ParseHex("0000106f20e665f741498cf23a4abfc5e52c675307ae7b28ea75cda72a48af89de0200000020714233a5a9c3a984c621ebdfd188dba90fccc47b162a648c1f2929d90c000000e68be583d1048006108729c02781c228afe9a1b785d067692054bb39d1658ddc");
  const std::vector<uint8_t> state_root = ParseHex("e68be583d1048006108729c02781c228afe9a1b785d067692054bb39d1658ddc");
  // clang-format on

  auto stateRoot = sha256twice(tx_root, pop_data_root);
  ASSERT_EQ(stateRoot.toHex(), HexStr(state_root));

  ContextInfoContainer ctx;
  ctx.height = height;
  ctx.keystones.firstPreviousKeystone = firstPreviousKeystone;
  ctx.keystones.secondPreviousKeystone = secondPreviousKeystone;

  AuthenticatedContextInfoContainer actx;
  actx.ctx = ctx;
  actx.stateRoot = stateRoot;
  ASSERT_EQ(SerializeToHex(actx), HexStr(pop_context_serialized));

  std::string expected =
      "756ecf78c55aa5b82bf475d5573fce78197e2312232a28241d3a2b2068331f02";
  ASSERT_EQ(actx.getTopLevelMerkleRoot().toHex(), expected);
}